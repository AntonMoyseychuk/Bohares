#include "pch.h"

#include "lexer.h"



typedef struct KeyWordToken
{
    const char* pLexeme;
    bohTokenType type;
} bohKeyWordToken;


static const bohKeyWordToken BOH_KEY_WORDS[] = {
    { "if",     TOKEN_TYPE_IF },
    { "then",   TOKEN_TYPE_THEN },
    { "else",   TOKEN_TYPE_ELSE },
    { "end",    TOKEN_TYPE_END },
    { "true",   TOKEN_TYPE_TRUE },
    { "false",  TOKEN_TYPE_FALSE },
    { "and",    TOKEN_TYPE_AND },
    { "or",     TOKEN_TYPE_OR },
    { "while",  TOKEN_TYPE_WHILE },
    { "do",     TOKEN_TYPE_DO },
    { "for",    TOKEN_TYPE_FOR },
    { "func",   TOKEN_TYPE_FUNC },
    { "null",   TOKEN_TYPE_NULL },
    { "print",  TOKEN_TYPE_PRINT },
    { "return", TOKEN_TYPE_RETURN },
};


enum
{
    BOH_TOKEN_SIZE_IN_BYTES = sizeof(bohToken),
    BOH_KEY_WORDS_COUNT = sizeof(BOH_KEY_WORDS) / sizeof(BOH_KEY_WORDS[0])
};


typedef bool (*storAlgorithmDelegate)(char ch);


static char lexPickCurrPosChar(bohLexer* pLexer)
{
    return pLexer->pData[pLexer->currPos];
}


static char lexPickNextNStepChar(bohLexer* pLexer, size_t n)
{
    const size_t nextNCharIdx = pLexer->currPos + n;

    return nextNCharIdx >= pLexer->dataSize ? '\0' : pLexer->pData[nextNCharIdx];
}


static char lexAdvanceCurrPos(bohLexer* pLexer)
{
    const size_t nextCharIdx = pLexer->currPos + 1;

    return nextCharIdx > pLexer->dataSize ? '\0' : pLexer->pData[pLexer->currPos++];
}


static char lexAdvanceCurrPosWhile(bohLexer* pLexer, storAlgorithmDelegate pFunc)
{
    char ch = lexPickCurrPosChar(pLexer);
    
    while(pFunc(ch)) {
        lexAdvanceCurrPos(pLexer);
        ch = lexPickCurrPosChar(pLexer);
    }
    
    return ch;
}


static bohKeyWordToken lexConvertIdentifierLexemeToKeyWord(const char* pLexemBegin, size_t lexemeLength)
{ 
    for (size_t i = 0; i < BOH_KEY_WORDS_COUNT; ++i) {
        const bohKeyWordToken* pKeyWord = BOH_KEY_WORDS + i;

        if (strncmp(pKeyWord->pLexeme, pLexemBegin, lexemeLength) == 0) {
            return *pKeyWord;
        }
    }

    bohKeyWordToken invalidKeyWordToken = { "unknown", TOKEN_TYPE_UNKNOWN };
    return invalidKeyWordToken;
}


static const char* lexConvertTokenTypeToStr(bohTokenType type)
{
    switch (type) {
        case TOKEN_TYPE_UNKNOWN: return "TOKEN_TYPE_UNKNOWN";
        case TOKEN_TYPE_COMMENT: return "TOKEN_TYPE_COMMENT";
        case TOKEN_TYPE_ASSIGN: return "TOKEN_TYPE_ASSIGN";
        case TOKEN_TYPE_LPAREN: return "TOKEN_TYPE_LPAREN";
        case TOKEN_TYPE_RPAREN: return "TOKEN_TYPE_RPAREN";
        case TOKEN_TYPE_LCURLY: return "TOKEN_TYPE_LCURLY";
        case TOKEN_TYPE_RCURLY: return "TOKEN_TYPE_RCURLY";
        case TOKEN_TYPE_LSQUAR: return "TOKEN_TYPE_LSQUAR";
        case TOKEN_TYPE_RSQUAR: return "TOKEN_TYPE_RSQUAR";
        case TOKEN_TYPE_COMMA: return "TOKEN_TYPE_COMMA";
        case TOKEN_TYPE_DOT: return "TOKEN_TYPE_DOT";
        case TOKEN_TYPE_PLUS: return "TOKEN_TYPE_PLUS";
        case TOKEN_TYPE_MINUS: return "TOKEN_TYPE_MINUS";
        case TOKEN_TYPE_MULT: return "TOKEN_TYPE_MULT";
        case TOKEN_TYPE_DIV: return "TOKEN_TYPE_DIV";
        case TOKEN_TYPE_MOD: return "TOKEN_TYPE_MOD";
        case TOKEN_TYPE_CARET: return "TOKEN_TYPE_CARET";
        case TOKEN_TYPE_COLON: return "TOKEN_TYPE_COLON";
        case TOKEN_TYPE_SEMICOLON: return "TOKEN_TYPE_SEMICOLON";
        case TOKEN_TYPE_QUESTION: return "TOKEN_TYPE_QUESTION";
        case TOKEN_TYPE_BITWISE_NOT: return "TOKEN_TYPE_BITWISE_NOT";
        case TOKEN_TYPE_NOT: return "TOKEN_TYPE_NOT";
        case TOKEN_TYPE_GREATER: return "TOKEN_TYPE_GREATER";
        case TOKEN_TYPE_LESS: return "TOKEN_TYPE_LESS";
        case TOKEN_TYPE_NOT_EQUAL: return "TOKEN_TYPE_NOT_EQUAL";
        case TOKEN_TYPE_GEQUAL: return "TOKEN_TYPE_GEQUAL";
        case TOKEN_TYPE_LEQUAL: return "TOKEN_TYPE_LEQUAL";
        case TOKEN_TYPE_EQUAL: return "TOKEN_TYPE_EQUAL";
        case TOKEN_TYPE_RSHIFT: return "TOKEN_TYPE_RSHIFT";
        case TOKEN_TYPE_LSHIFT: return "TOKEN_TYPE_LSHIFT";
        case TOKEN_TYPE_IDENTIFIER: return "TOKEN_TYPE_IDENTIFIER";
        case TOKEN_TYPE_STRING: return "TOKEN_TYPE_STRING";
        case TOKEN_TYPE_INTEGER: return "TOKEN_TYPE_INTEGER";
        case TOKEN_TYPE_FLOAT: return "TOKEN_TYPE_FLOAT";
        case TOKEN_TYPE_IF: return "TOKEN_TYPE_IF";
        case TOKEN_TYPE_THEN: return "TOKEN_TYPE_THEN";
        case TOKEN_TYPE_ELSE: return "TOKEN_TYPE_ELSE";
        case TOKEN_TYPE_END: return "TOKEN_TYPE_END";
        case TOKEN_TYPE_TRUE: return "TOKEN_TYPE_TRUE";
        case TOKEN_TYPE_FALSE: return "TOKEN_TYPE_FALSE";
        case TOKEN_TYPE_AND: return "TOKEN_TYPE_AND";
        case TOKEN_TYPE_OR: return "TOKEN_TYPE_OR";
        case TOKEN_TYPE_WHILE: return "TOKEN_TYPE_WHILE";
        case TOKEN_TYPE_DO: return "TOKEN_TYPE_DO";
        case TOKEN_TYPE_FOR: return "TOKEN_TYPE_FOR";
        case TOKEN_TYPE_FUNC: return "TOKEN_TYPE_FUNC";
        case TOKEN_TYPE_NULL: return "TOKEN_TYPE_NULL";
        case TOKEN_TYPE_PRINT: return "TOKEN_TYPE_PRINT";
        case TOKEN_TYPE_RETURN: return "TOKEN_TYPE_RETURN";
    
    default: 
        assert(false && "Invalid token type"); 
        return "INVALID";
    }
}


static bool IsEndLineChar(char ch)
{
    return ch == '\n' || ch == '\0';
}


static bool IsNotEndLineChar(char ch)
{
    return !IsEndLineChar(ch);
}


static bool IsNotDoubleQuoteOrEndChar(char ch)
{
    return ch != '\"' && ch != '\0';
}


static bool IsNotMultilineCommentEndOrEndChar(char ch)
{
    return ch != ']' && ch != '\0';
}


static bool IsDigitChar(char ch)
{
    return ch >= '0' && ch <= '9';
}


static bool IsAlphaChar(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}


static bool IsUnderscoreChar(char ch)
{
    return ch == '_';
}


static bool IsIdentifierAppropriateChar(char ch)
{
    return IsAlphaChar(ch) || IsUnderscoreChar(ch) || IsDigitChar(ch);
}


static void storTokenStorageEmplaceBack(bohTokenStorage* pStorage, const char* pLexeme, size_t lexemeLength, bohTokenType type, uint32_t line, uint32_t column)
{
    assert(pStorage);

    bohToken token = bohTokenCreateLexemeSized(pLexeme, lexemeLength, type, line, column);

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

    token.lexeme = bohStringCreateCStr(pLexeme);
    token.type = type;
    token.line = line;
    token.column = column;

    return token;
}


bohToken bohTokenCreateLexemeSized(const char* pLexemeBegin, size_t lexemeLength, bohTokenType type, uint32_t line, uint32_t column)
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


bohTokenType bohTokenGetType(const bohToken *pToken)
{
    assert(pToken);
    return pToken->type;
}


const char* bohTokenGetTypeStr(const bohToken *pToken)
{
    assert(pToken);
    return lexConvertTokenTypeToStr(pToken->type);
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
        ++pLexer->column;

        const char ch = lexAdvanceCurrPos(pLexer);

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
                if (lexPickCurrPosChar(pLexer) == '[') {
                    char currCh = lexAdvanceCurrPosWhile(pLexer, IsNotMultilineCommentEndOrEndChar);
                    assert(currCh == ']' && "Missed closing multiline comment symbol ]. Make it compile error");

                    lexAdvanceCurrPos(pLexer); // Consume ']' symbol
                    currCh = lexPickCurrPosChar(pLexer);
                    assert(currCh == '#' && "Missed closing multiline comment symbol #. Make it compile error");

                    lexAdvanceCurrPos(pLexer); // Consume '#' symbol
                } else {
                    lexAdvanceCurrPosWhile(pLexer, IsNotEndLineChar);
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
                switch (lexPickCurrPosChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrPos(pLexer);
                        type = TOKEN_TYPE_NOT_EQUAL;
                        break;
                    default: 
                        type = TOKEN_TYPE_NOT;
                        break;
                }
                
                break;
            case '=':
                switch (lexPickCurrPosChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrPos(pLexer);
                        type = TOKEN_TYPE_EQUAL;
                        break;
                    default: 
                        type = TOKEN_TYPE_ASSIGN;
                        break;
                }
                
                break;
            case '>':
                switch (lexPickCurrPosChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrPos(pLexer);
                        type = TOKEN_TYPE_GEQUAL;
                        break;
                    case '>':
                        lexAdvanceCurrPos(pLexer);
                        type = TOKEN_TYPE_RSHIFT;
                        break;
                    default: 
                        type = TOKEN_TYPE_GREATER;
                        break;
                }

                break;
            case '<':
                switch (lexPickCurrPosChar(pLexer)) {
                    case '=':
                        lexAdvanceCurrPos(pLexer);
                        type = TOKEN_TYPE_LEQUAL;
                        break;
                    case '<':
                        lexAdvanceCurrPos(pLexer);
                        type = TOKEN_TYPE_LSHIFT;
                        break;
                    default: 
                        type = TOKEN_TYPE_LESS;
                        break;
                }

                break;
            case '\"':
            {
                const char currCh = lexAdvanceCurrPosWhile(pLexer, IsNotDoubleQuoteOrEndChar);
                assert(currCh == '\"' && "Missed closing double quotes. Make it compile error");

                lexAdvanceCurrPos(pLexer); // Consume '"' symbol

                type = TOKEN_TYPE_STRING;
                break;
            }
            default:
                type = TOKEN_TYPE_UNKNOWN;
                break;
        }

        if (type == TOKEN_TYPE_UNKNOWN) {
            if (IsDigitChar(ch)) {
                lexAdvanceCurrPosWhile(pLexer, IsDigitChar);

                type = TOKEN_TYPE_INTEGER;

                if (lexPickCurrPosChar(pLexer) == '.') {
                    const char nextCh = lexPickNextNStepChar(pLexer, 1);
                    assert(IsDigitChar(nextCh) && "Invalid floating point number gramma. Make it compile error");
                    
                    lexAdvanceCurrPos(pLexer); // Consume the '.'
                    
                    lexAdvanceCurrPosWhile(pLexer, IsDigitChar);

                    type = TOKEN_TYPE_FLOAT;
                }
            }
        }

        if (type == TOKEN_TYPE_UNKNOWN) {
            if (IsAlphaChar(ch) || IsUnderscoreChar(ch)) {
                lexAdvanceCurrPosWhile(pLexer, IsIdentifierAppropriateChar);
                type = TOKEN_TYPE_IDENTIFIER;
            }
        }

        assert(type != TOKEN_TYPE_UNKNOWN && "Undefined token");

        const char* pLexemBegin = pLexer->pData + pLexer->startPos;
        const size_t lexemeLength = pLexer->currPos - pLexer->startPos;

        if (type == TOKEN_TYPE_IDENTIFIER) {
            const bohKeyWordToken keyWord = lexConvertIdentifierLexemeToKeyWord(pLexemBegin, lexemeLength);

            if (keyWord.type != TOKEN_TYPE_UNKNOWN) {
                type = keyWord.type;
            }
        }

        storTokenStorageEmplaceBack(&tokens, pLexemBegin, lexemeLength, type, pLexer->line, pLexer->column);
    }

    return tokens;
}


bool bohLexerIsValid(bohLexer* pLexer)
{
    return pLexer && pLexer->pData != NULL && pLexer->dataSize > 0;
}
