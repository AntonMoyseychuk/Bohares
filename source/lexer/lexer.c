#include "pch.h"

#include "lexer.h"


#define BOH_MAX_TOKEN_LEXEME_LENGTH 15
#define BOH_NULL_TERMINATED_MAX_TOKEN_LEXEME_LENGTH (BOH_MAX_TOKEN_LEXEME_LENGTH + 1)


enum
{
    BOH_TOKEN_SIZE_IN_BYTES = sizeof(bohToken)
};

static_assert(BOH_TOKEN_SIZE_IN_BYTES == 12, "Invalid bohToken size");


typedef struct TokenTypeLexemMapping
{
    bohTokenType type;
    char lexeme[BOH_MAX_TOKEN_LEXEME_LENGTH + 1];
} bohTokenTypeLexemMapping;


static const bohTokenTypeLexemMapping BOH_TOKEN_TO_LEXEM_MAP[] = {
    { TOKEN_TYPE_UNKNOWN, "unknown" },
    { TOKEN_TYPE_COMMENT, "#" },
    { TOKEN_TYPE_ASSIGN, "=" },
    { TOKEN_TYPE_LPAREN, "(" },
    { TOKEN_TYPE_RPAREN, ")" },
    { TOKEN_TYPE_LCURLY, "{" },
    { TOKEN_TYPE_RCURLY, "}" },
    { TOKEN_TYPE_LSQUAR, "[" },
    { TOKEN_TYPE_RSQUAR, "]" },
    { TOKEN_TYPE_COMMA, "," },
    { TOKEN_TYPE_DOT, "." },
    { TOKEN_TYPE_PLUS, "+" },
    { TOKEN_TYPE_MINUS, "-" },
    { TOKEN_TYPE_MULT, "*" },
    { TOKEN_TYPE_DIV, "/" },
    { TOKEN_TYPE_MOD, "%" },
    { TOKEN_TYPE_CARET, "^" },
    { TOKEN_TYPE_COLON, ":" },
    { TOKEN_TYPE_SEMICOLON, ";" },
    { TOKEN_TYPE_QUESTION, "?" },
    { TOKEN_TYPE_BITWISE_NOT, "~" },
    { TOKEN_TYPE_NOT, "!" },
    { TOKEN_TYPE_GREATER, ">" },
    { TOKEN_TYPE_LESS, "<" },
    { TOKEN_TYPE_NOT_EQUAL, "!=" },
    { TOKEN_TYPE_GEQUAL, ">=" },
    { TOKEN_TYPE_LEQUAL, "<=" },
    { TOKEN_TYPE_EQUAL, "==" },
    { TOKEN_TYPE_RSHIFT, ">>" },
    { TOKEN_TYPE_LSHIFT, "<<" },

    // Literals
    { TOKEN_TYPE_IDENTIFIER, "identifier" },
    { TOKEN_TYPE_STRING, "string" },
    { TOKEN_TYPE_INTEGER, "integer" },
    { TOKEN_TYPE_FLOAT, "float" },
    
    // Keywords
    { TOKEN_TYPE_IF, "if" },
    { TOKEN_TYPE_THEN, "then" },
    { TOKEN_TYPE_ELSE, "else" },
    { TOKEN_TYPE_END, "end" },
    { TOKEN_TYPE_TRUE, "true" },
    { TOKEN_TYPE_FALSE, "false" },
    { TOKEN_TYPE_AND, "and" },
    { TOKEN_TYPE_OR, "or" },
    { TOKEN_TYPE_WHILE, "while" },
    { TOKEN_TYPE_DO, "do" },
    { TOKEN_TYPE_FOR, "for" },
    { TOKEN_TYPE_FUNC, "func" },
    { TOKEN_TYPE_NULL, "null" },
    { TOKEN_TYPE_PRINT, "print" },
    { TOKEN_TYPE_PRINTLN, "println" },
    { TOKEN_TYPE_RETURN, "return" },
};


static char lexAdvanceCurrChar(bohLexer* pLexer)
{
    const size_t nextCharIdx = pLexer->currPos + 1;

    return nextCharIdx > pLexer->dataSize ? '\0' : pLexer->pData[pLexer->currPos++];
}


static char lexPickCurrChar(bohLexer* pLexer)
{
    return pLexer->pData[pLexer->currPos];
}


static char lexPickNextCharN(bohLexer* pLexer, size_t n)
{
    const size_t nextNCharIdx = pLexer->currPos + n;

    return nextNCharIdx >= pLexer->dataSize ? '\0' : pLexer->pData[pLexer->currPos];
}


static bool lexIsEndLineChar(char ch)
{
    return ch == '\n' || ch == '\0';
}


static bool lexIsDigitChar(char ch)
{
    return ch >= '0' && ch <= '9';
}


static void lexAddTokenToStorage(bohTokenStorage* pStorage, bohTokenType type, uint32_t line, uint32_t column)
{
    assert(pStorage);

    bohToken token = bohTokenCreate(type, line, column);

    bohTokenStoragePushBack(pStorage, &token);
}


bohToken bohTokenCreate(bohTokenType type, uint32_t line, uint32_t column)
{
    bohToken token;

    token.type = type;
    token.line = line;
    token.column = column;

    return token;
}


const char* bohTokenGetLexeme(const bohToken* pToken)
{
    assert(pToken);

    return BOH_TOKEN_TO_LEXEM_MAP[pToken->type].lexeme;
}


bohTokenStorage bohTokenStorageCreate(void)
{
    bohTokenStorage storage;
    storage.pTokens = NULL;
    storage.size = 0;
    storage.capacity = 0;

    return storage;
}


void bohTokenStorageDestroy(bohTokenStorage* pStorage)
{
    assert(pStorage);

    free(pStorage->pTokens);
    pStorage->size = 0;
    pStorage->capacity = 0;
}


void bohTokenStorageReserve(bohTokenStorage* pStorage, size_t newCapacity)
{
    assert(pStorage);

    if (newCapacity <= pStorage->capacity) {
        return;
    }

    const size_t newCapacityInBytes = newCapacity * BOH_TOKEN_SIZE_IN_BYTES;

    bohToken* pNewTokensBuffer = (bohToken*)malloc(newCapacityInBytes);
    memset(pNewTokensBuffer, 0, newCapacityInBytes);
    assert(pNewTokensBuffer);

    if (pStorage->size > 0) {
        memcpy_s(pNewTokensBuffer, newCapacityInBytes, pStorage->pTokens, pStorage->size * BOH_TOKEN_SIZE_IN_BYTES);
    }
    
    bohToken* pOldTokensBuffer = pStorage->pTokens;
    pStorage->pTokens = pNewTokensBuffer;
    pStorage->capacity = newCapacity;

    free(pOldTokensBuffer);
}


void bohTokenStorageResize(bohTokenStorage* pStorage, size_t newSize)
{
    assert(pStorage);

    if (newSize <= pStorage->size) {
        pStorage->size = newSize;
        return;
    }

    bohTokenStorageReserve(pStorage, newSize);
    pStorage->size = newSize;
}


void bohTokenStoragePushBack(bohTokenStorage* pStorage, const bohToken* pToken)
{
    assert(pStorage);
    assert(pToken);

    const size_t currSize = bohTokenStorageGetSize(pStorage); 
    const size_t currCapacity = bohTokenStorageGetCapacity(pStorage); 

    if (currSize + 1 > currCapacity) {
        const bool isZeroSized = currSize == 0;

        const size_t newSize = (currSize + (size_t)isZeroSized) * 2;
        bohTokenStorageReserve(pStorage, newSize);
    }

    pStorage->pTokens[currSize] = *pToken;
    ++pStorage->size;
}


bohToken* bohTokenStorageAt(bohTokenStorage* pStorage, size_t index)
{
    assert(pStorage);
    assert(index < pStorage->size);
    
    return &pStorage->pTokens[index];
}


size_t bohTokenStorageGetSize(const bohTokenStorage* pStorage)
{
    assert(pStorage);
    
    return pStorage->size;
}


size_t bohTokenStorageGetCapacity(const bohTokenStorage *pStorage)
{
    assert(pStorage);
    
    return pStorage->capacity;
}


bool bohTokenStorageIsEmpty(const bohTokenStorage* pStorage)
{
    assert(pStorage);
    return pStorage->size == 0 || pStorage->pTokens == NULL;
}


bohLexer bohLexerCreate(const char* pCodeData, size_t codeDataSize)
{
    assert(pCodeData);

    bohLexer lexer;

    lexer.pData = pCodeData;
    lexer.dataSize = codeDataSize;

    lexer.startPos = 0;
    lexer.currPos = 0;
    
    lexer.line = 1;
    lexer.column = 0;
    
    return lexer;
}


void bohLexerDestroy(bohLexer* pLexer)
{
    assert(pLexer);

    pLexer->pData = NULL;
    pLexer->dataSize = 0;

    pLexer->startPos = 0;
    pLexer->currPos = 0;
    
    pLexer->line = 1;
    pLexer->column = 0;
}


bohTokenStorage bohLexerTokenize(bohLexer* pLexer)
{
    assert(pLexer);
    assert(bohLexerIsValid(pLexer));

    bohTokenStorage tokens = bohTokenStorageCreate();

    const size_t dataSize = pLexer->dataSize;
    
    while (pLexer->currPos < dataSize) {
        pLexer->startPos = pLexer->currPos;

        const char ch = lexAdvanceCurrChar(pLexer);

        ++pLexer->column;

        bohTokenType type = TOKEN_TYPE_UNKNOWN;

        switch (ch) {
            case '\n':
                ++pLexer->line;
                pLexer->column = 1;
            case '\t':
            case '\r':
            case ' ':
            case '\0':
                continue;

            case '#':
                while(!lexIsEndLineChar(lexPickCurrChar(pLexer))) {
                    lexAdvanceCurrChar(pLexer);
                }
                continue;

            case '(': type = TOKEN_TYPE_LPAREN; break;
            case ')': type = TOKEN_TYPE_RPAREN; break;
            case '{': type = TOKEN_TYPE_LCURLY; break;
            case '}': type = TOKEN_TYPE_RCURLY; break;
            case '[': type = TOKEN_TYPE_LSQUAR; break;
            case ']': type = TOKEN_TYPE_RSQUAR; break;
            case ',': type = TOKEN_TYPE_COMMA; break;
            case '.': type = TOKEN_TYPE_DOT; break;
            case '+': type = TOKEN_TYPE_PLUS; break;
            case '-': type = TOKEN_TYPE_MINUS; break;
            case '*': type = TOKEN_TYPE_MULT; break;
            case '/': type = TOKEN_TYPE_DIV; break;
            case '%': type = TOKEN_TYPE_MOD; break;
            case '^': type = TOKEN_TYPE_CARET; break;
            case ':': type = TOKEN_TYPE_COLON; break;
            case ';': type = TOKEN_TYPE_SEMICOLON; break;
            case '?': type = TOKEN_TYPE_QUESTION; break;
            case '~': type = TOKEN_TYPE_BITWISE_NOT; break;
            case '!': 
                switch (lexPickCurrChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrChar(pLexer);
                        type = TOKEN_TYPE_NOT_EQUAL;
                        break;
                    default: 
                        type = TOKEN_TYPE_NOT;
                        break;
                }
                
                break;
            case '=':
                switch (lexPickCurrChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrChar(pLexer);
                        type = TOKEN_TYPE_EQUAL;
                        break;
                    default: 
                        type = TOKEN_TYPE_ASSIGN;
                        break;
                }
                
                break;
            case '>':
                switch (lexPickCurrChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrChar(pLexer);
                        type = TOKEN_TYPE_GEQUAL;
                        break;
                    case '>':
                        lexAdvanceCurrChar(pLexer);
                        type = TOKEN_TYPE_RSHIFT;
                        break;
                    default: 
                        type = TOKEN_TYPE_GREATER;
                        break;
                }

                break;
            case '<':
                switch (lexPickCurrChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrChar(pLexer);
                        type = TOKEN_TYPE_LEQUAL;
                        break;
                    case '<':
                        lexAdvanceCurrChar(pLexer);
                        type = TOKEN_TYPE_LSHIFT;
                        break;
                    default: 
                        type = TOKEN_TYPE_LESS;
                        break;
                }

                break;
            default:
                type = TOKEN_TYPE_UNKNOWN;
                break;
        }

        lexAddTokenToStorage(&tokens, type, pLexer->line, pLexer->column);
    }

    return tokens;
}


bool bohLexerIsValid(bohLexer* pLexer)
{
    return pLexer && pLexer->pData != NULL && pLexer->dataSize > 0;
}
