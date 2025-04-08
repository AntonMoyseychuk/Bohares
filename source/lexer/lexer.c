#include "pch.h"

#include "lexer.h"


enum
{
    BOH_TOKEN_SIZE_IN_BYTES = sizeof(bohToken)
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


static void lexAddTokenToStorage(bohTokenStorage* pStorage, const char* pLexeme, size_t lexemeLength, bohTokenType type, uint32_t line, uint32_t column)
{
    assert(pStorage);

    bohToken token = bohTokenCreateLexemSized(pLexeme, lexemeLength, type, line, column);

    bohTokenStoragePushBack(pStorage, &token);
}


bohToken bohTokenCreateDefault(void)
{
    bohToken token;

    token.lexeme = bohStringCreate();
    token.type = TOKEN_TYPE_UNKNOWN;
    token.line = 0;
    token.column = 0;

    return token;
}


bohToken bohTokenCreate(const char *pLexeme, bohTokenType type, uint32_t line, uint32_t column)
{
    bohToken token;

    token.lexeme = bohStringCreateStr(pLexeme);
    token.type = type;
    token.line = line;
    token.column = column;

    return token;
}


bohToken bohTokenCreateLexemSized(const char* pLexemeBegin, size_t lexemeLength, bohTokenType type, uint32_t line, uint32_t column)
{
    bohToken token;

    const char* pLexemeEnd = pLexemeBegin + lexemeLength;

    token.lexeme = bohStringCreateFromTo(pLexemeBegin, pLexemeEnd);
    token.type = type;
    token.line = line;
    token.column = column;

    return token;
}


void bohTokenDestroy(bohToken* pToken)
{
    assert(pToken);

    bohStringDestroy(&pToken->lexeme);
    pToken->type = TOKEN_TYPE_UNKNOWN;
    pToken->line = 0;
    pToken->column = 0;
}


void bohTokenAssign(bohToken* pDst, const bohToken* pSrc)
{
    assert(pDst);
    assert(pSrc);

    bohStringAssign(&pDst->lexeme, &pSrc->lexeme);
    pDst->type = pSrc->type;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;
}


const bohString* bohTokenGetLexeme(const bohToken* pToken)
{
    assert(pToken);

    return &pToken->lexeme;
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

    bohTokenStorageResize(pStorage, 0);

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

    const size_t size = pStorage->size;
    bohToken* pOldTokensBuffer = pStorage->pTokens;

    for (size_t i = 0; i < size; ++i) {
        bohTokenAssign(pNewTokensBuffer + i, pOldTokensBuffer + i);
    }
    
    pStorage->pTokens = pNewTokensBuffer;
    pStorage->capacity = newCapacity;

    free(pOldTokensBuffer);
}


void bohTokenStorageResize(bohTokenStorage* pStorage, size_t newSize)
{
    assert(pStorage);

    const size_t oldSize = pStorage->size;

    if (newSize == oldSize) {
        return;
    }

    if (newSize < oldSize) {
        for (size_t i = newSize; i < oldSize; ++i) {
            bohToken* pToken = pStorage->pTokens + i;
            bohTokenDestroy(pToken);
        }

        pStorage->size = newSize;

        return;
    }

    bohTokenStorageReserve(pStorage, newSize);
    for (size_t i = oldSize; i < newSize; ++i) {
        pStorage->pTokens[i] = bohTokenCreateDefault();
    }

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

    bohTokenAssign(pStorage->pTokens + currSize, pToken);
    ++pStorage->size;
}


bohToken* bohTokenStorageAt(bohTokenStorage* pStorage, size_t index)
{
    assert(pStorage);
    assert(index < pStorage->size);
    
    return pStorage->pTokens + index;
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

        const char* pLexemBegin = type != TOKEN_TYPE_UNKNOWN ? pLexer->pData + pLexer->startPos : "unknown";
        const size_t lexemeLength = type != TOKEN_TYPE_UNKNOWN ? pLexer->currPos - pLexer->startPos : strlen(pLexemBegin);

        lexAddTokenToStorage(&tokens, pLexemBegin, lexemeLength, type, pLexer->line, pLexer->column);
    }

    return tokens;
}


bool bohLexerIsValid(bohLexer* pLexer)
{
    return pLexer && pLexer->pData != NULL && pLexer->dataSize > 0;
}
