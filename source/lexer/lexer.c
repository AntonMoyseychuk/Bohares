#include "pch.h"

#include "lexer.h"
#include "utils/message/message.h"


#define BOH_THROW_LEXER_ERROR(LINE, COLUMN, FMT, ...)                                           \
{                                                                                               \
    fprintf_s(stderr, "[BOH LEXER ERROR] (%u, %u): ", (uint64_t)(LINE), (uint64_t)(COLUMN));    \
    BOH_THROW_ERROR_FMT(FMT, __VA_ARGS__);                                                      \
}

#define BOH_CHECK_LEXER_COND(COND, LINE, COLUMN, FMT, ...)     \
    if (!(COND)) {                                             \
        BOH_THROW_LEXER_ERROR(LINE, COLUMN, FMT, __VA_ARGS__); \
    }


typedef struct KeyWordToken
{
    bohStringView lexeme;
    bohTokenType type;
} bohKeyWordToken;


typedef bool (*storAlgorithmDelegate)(char ch);


#define BOH_CREATE_KEY_WORD_TOKEN(lexeme, type) { { lexeme, sizeof(lexeme) - 1 }, type }

static const bohKeyWordToken BOH_KEY_WORDS[] = {
    BOH_CREATE_KEY_WORD_TOKEN("if",     BOH_TOKEN_TYPE_IF),
    BOH_CREATE_KEY_WORD_TOKEN("then",   BOH_TOKEN_TYPE_THEN),
    BOH_CREATE_KEY_WORD_TOKEN("else",   BOH_TOKEN_TYPE_ELSE),
    BOH_CREATE_KEY_WORD_TOKEN("end",    BOH_TOKEN_TYPE_END),
    BOH_CREATE_KEY_WORD_TOKEN("true",   BOH_TOKEN_TYPE_TRUE),
    BOH_CREATE_KEY_WORD_TOKEN("false",  BOH_TOKEN_TYPE_FALSE),
    BOH_CREATE_KEY_WORD_TOKEN("and",    BOH_TOKEN_TYPE_AND),
    BOH_CREATE_KEY_WORD_TOKEN("or",     BOH_TOKEN_TYPE_OR),
    BOH_CREATE_KEY_WORD_TOKEN("while",  BOH_TOKEN_TYPE_WHILE),
    BOH_CREATE_KEY_WORD_TOKEN("do",     BOH_TOKEN_TYPE_DO),
    BOH_CREATE_KEY_WORD_TOKEN("for",    BOH_TOKEN_TYPE_FOR),
    BOH_CREATE_KEY_WORD_TOKEN("func",   BOH_TOKEN_TYPE_FUNC),
    BOH_CREATE_KEY_WORD_TOKEN("null",   BOH_TOKEN_TYPE_NULL),
    BOH_CREATE_KEY_WORD_TOKEN("print",  BOH_TOKEN_TYPE_PRINT),
    BOH_CREATE_KEY_WORD_TOKEN("return", BOH_TOKEN_TYPE_RETURN),
};

static const size_t BOH_KEY_WORDS_COUNT = sizeof(BOH_KEY_WORDS) / sizeof(BOH_KEY_WORDS[0]);


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


static void lexTokenDefConstructor(void* pToken)
{
    assert(pToken);
    *((bohToken*)pToken) = bohTokenCreate();
}


static void lexTokenDestructor(void* pToken)
{
    assert(pToken);
    bohTokenDestroy((bohToken*)pToken);
}


static void lexTokenCopy(void* pDstToken, const void* pSrcToken)
{
    assert(pDstToken);
    assert(pSrcToken);
    bohTokenAssign((bohToken*)pDstToken, (const bohToken*)pSrcToken);
}


static char lexPickCurrPosChar(bohLexer* pLexer)
{
    return bohStringViewAt(&pLexer->data, pLexer->currPos);
}


static char lexPickNextNStepChar(bohLexer* pLexer, size_t n)
{
    const size_t nextNCharIdx = pLexer->currPos + n;
    const size_t dataSize = bohStringViewGetSize(&pLexer->data);

    return nextNCharIdx >= dataSize ? '\0' : bohStringViewAt(&pLexer->data, nextNCharIdx);
}


static char lexAdvanceCurrPos(bohLexer* pLexer)
{
    const size_t nextCharIdx = pLexer->currPos + 1;
    const size_t dataSize = bohStringViewGetSize(&pLexer->data);

    pLexer->column = nextCharIdx > dataSize ? pLexer->column : pLexer->column + 1;

    return nextCharIdx > dataSize ? '\0' : bohStringViewAt(&pLexer->data, pLexer->currPos++);
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


static bohKeyWordToken lexConvertIdentifierLexemeToKeyWord(bohStringView tokenLexeme)
{ 
    for (size_t i = 0; i < BOH_KEY_WORDS_COUNT; ++i) {
        const bohKeyWordToken* pKeyWord = BOH_KEY_WORDS + i;

        if (bohStringViewEqual(&pKeyWord->lexeme, &tokenLexeme)) {
            return *pKeyWord;
        }
    }

    bohKeyWordToken invalidKeyWordToken = BOH_CREATE_KEY_WORD_TOKEN("unknown", BOH_TOKEN_TYPE_UNKNOWN);
    return invalidKeyWordToken;
}


static bohStringView lexGetCurrLexem(bohLexer* pLexer)
{
    const char* pLexemBegin = bohStringViewGetData(&pLexer->data) + pLexer->startPos;
    const size_t lexemeLength = pLexer->currPos - pLexer->startPos;
    
    return bohStringViewCreateCStrSized(pLexemBegin, lexemeLength);
}


static bohToken lexGetNextToken(bohLexer* pLexer)
{
    assert(pLexer);

    pLexer->startPos = pLexer->currPos;

    const size_t tokenLine = pLexer->line;
    const size_t tokenColumn = pLexer->column;

    const char ch = lexAdvanceCurrPos(pLexer);

    bohTokenType type = BOH_TOKEN_TYPE_UNKNOWN;

    switch (ch) {
        case '\n':
            ++pLexer->line;
            pLexer->column = 0;
            type = BOH_TOKEN_TYPE_DUMMY;
            break;
        case '\r': 
            pLexer->column = 0;
            type = BOH_TOKEN_TYPE_DUMMY;
            break;
        case '\t': type = BOH_TOKEN_TYPE_DUMMY; break;
        case '\0': type = BOH_TOKEN_TYPE_DUMMY; break;
        case ' ': type = BOH_TOKEN_TYPE_DUMMY; break;
        case '(': type = BOH_TOKEN_TYPE_LPAREN; break;
        case ')': type = BOH_TOKEN_TYPE_RPAREN; break;
        case '{': type = BOH_TOKEN_TYPE_LCURLY; break;
        case '}': type = BOH_TOKEN_TYPE_RCURLY; break;
        case '[': type = BOH_TOKEN_TYPE_LSQUAR; break;
        case ']': type = BOH_TOKEN_TYPE_RSQUAR; break;
        case ',': type = BOH_TOKEN_TYPE_COMMA; break;
        case '.': type = BOH_TOKEN_TYPE_DOT; break;
        case '+': type = BOH_TOKEN_TYPE_PLUS; break;
        case '-': type = BOH_TOKEN_TYPE_MINUS; break;
        case '*': type = BOH_TOKEN_TYPE_MULT; break;
        case '/': type = BOH_TOKEN_TYPE_DIV; break;
        case '%': type = BOH_TOKEN_TYPE_MOD; break;
        case '^': type = BOH_TOKEN_TYPE_XOR; break;
        case ':': type = BOH_TOKEN_TYPE_COLON; break;
        case ';': type = BOH_TOKEN_TYPE_SEMICOLON; break;
        case '?': type = BOH_TOKEN_TYPE_QUESTION; break;
        case '~': type = BOH_TOKEN_TYPE_BITWISE_NOT; break;
        case '!': 
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_NOT_EQUAL;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_NOT;
                    break;
            }
                
            break;
        case '=':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_EQUAL;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_ASSIGN;
                    break;
            }
                
            break;
        case '>':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_GEQUAL;
                    break;
                case '>':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_RSHIFT;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_GREATER;
                    break;
            }

            break;
        case '<':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_LEQUAL;
                    break;
                case '<':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_LSHIFT;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_LESS;
                    break;
            }

            break;
        case '\"':
        {
            const char currCh = lexAdvanceCurrPosWhile(pLexer, IsNotDoubleQuoteOrEndChar);
            BOH_CHECK_LEXER_COND(currCh == '\"', pLexer->line, pLexer->column, "missed closing double quotes");

            lexAdvanceCurrPos(pLexer); // Consume '"' symbol

            type = BOH_TOKEN_TYPE_STRING;
            break;
        }
        case '#':
            if (lexPickCurrPosChar(pLexer) == '[') {
                char currCh = lexAdvanceCurrPosWhile(pLexer, IsNotMultilineCommentEndOrEndChar);
                BOH_CHECK_LEXER_COND(currCh == ']', pLexer->line, pLexer->column, "missed closing multiline comment symbol, expected \']#\'");

                lexAdvanceCurrPos(pLexer); // Consume ']' symbol
                
                currCh = lexPickCurrPosChar(pLexer);
                BOH_CHECK_LEXER_COND(currCh == '#', pLexer->line, pLexer->column, "missed closing multiline comment symbol, expected \']#\'");

                lexAdvanceCurrPos(pLexer); // Consume '#' symbol
            } else {
                lexAdvanceCurrPosWhile(pLexer, IsNotEndLineChar);
            }

            type = BOH_TOKEN_TYPE_DUMMY;
            break;
        default:
            break;
    }

    // Number
    if (type == BOH_TOKEN_TYPE_UNKNOWN && IsDigitChar(ch)) {
        lexAdvanceCurrPosWhile(pLexer, IsDigitChar);

        type = BOH_TOKEN_TYPE_INTEGER;

        if (lexPickCurrPosChar(pLexer) == '.') {
            const char nextCh = lexPickNextNStepChar(pLexer, 1);
            BOH_CHECK_LEXER_COND(IsDigitChar(nextCh), pLexer->line, pLexer->column, "invalid floating point number grammar");
            
            lexAdvanceCurrPos(pLexer); // Consume the '.'
                
            lexAdvanceCurrPosWhile(pLexer, IsDigitChar);

            type = BOH_TOKEN_TYPE_FLOAT;
        }
    }

    // Key word or identifier
    if (type == BOH_TOKEN_TYPE_UNKNOWN && (IsAlphaChar(ch) || IsUnderscoreChar(ch))) {
        lexAdvanceCurrPosWhile(pLexer, IsIdentifierAppropriateChar);
        
        const bohStringView lexeme = lexGetCurrLexem(pLexer);
        const bohKeyWordToken keyWord = lexConvertIdentifierLexemeToKeyWord(lexeme);

        type = keyWord.type != BOH_TOKEN_TYPE_UNKNOWN ? keyWord.type : BOH_TOKEN_TYPE_IDENTIFIER;
    }

    return bohTokenCreateParams(lexGetCurrLexem(pLexer), type, tokenLine, tokenColumn);
}


bohToken bohTokenCreate(void)
{
    bohToken token;

    token.lexeme = bohStringViewCreate();
    token.type = BOH_TOKEN_TYPE_UNKNOWN;
    token.line = 0;
    token.column = 0;

    return token;
}


bohToken bohTokenCreateParams(bohStringView lexeme, bohTokenType type, uint32_t line, uint32_t column)
{
    bohToken token;

    bohStringViewAssign(&token.lexeme, &lexeme);
    token.type = type;
    token.line = line;
    token.column = column;

    return token;
}


void bohTokenDestroy(bohToken* pToken)
{
    assert(pToken);

    bohStringViewReset(&pToken->lexeme);
    pToken->type = BOH_TOKEN_TYPE_UNKNOWN;
    pToken->line = 0;
    pToken->column = 0;
}


void bohTokenAssign(bohToken* pDst, const bohToken* pSrc)
{
    assert(pDst);
    assert(pSrc);

    bohStringViewAssign(&pDst->lexeme, &pSrc->lexeme);
    pDst->type = pSrc->type;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;
}


bohStringView bohTokenGetLexeme(const bohToken* pToken)
{
    assert(pToken);
    return pToken->lexeme;
}


bohTokenType bohTokenGetType(const bohToken *pToken)
{
    assert(pToken);
    return pToken->type;
}


const char* bohTokenGetTypeStr(const bohToken *pToken)
{
    assert(pToken);
    return bohLexerConvertTokenTypeToStr(pToken->type);
}


bohLexer bohLexerCreate(const char* pCodeData, size_t codeDataSize)
{
    assert(pCodeData);

    bohLexer lexer;

    lexer.data = bohStringViewCreateCStrSized(pCodeData, codeDataSize);

    lexer.startPos = 0;
    lexer.currPos = 0;
    
    lexer.line = 1;
    lexer.column = 0;
    
    return lexer;
}


void bohLexerDestroy(bohLexer* pLexer)
{
    assert(pLexer);

    bohStringViewReset(&pLexer->data);

    pLexer->startPos = 0;
    pLexer->currPos = 0;
    
    pLexer->line = 1;
    pLexer->column = 0;
}


bohTokenStorage bohLexerTokenize(bohLexer* pLexer)
{
    assert(pLexer);
    assert(bohLexerIsValid(pLexer));

    bohTokenStorage tokens = BOH_DYN_ARRAY_CREATE(bohToken, lexTokenDefConstructor, lexTokenDestructor, lexTokenCopy);

    const size_t dataSize = bohStringViewGetSize(&pLexer->data);
    
    while (pLexer->currPos < dataSize) {
        const bohToken token = lexGetNextToken(pLexer);
        BOH_CHECK_LEXER_COND(token.type != BOH_TOKEN_TYPE_UNKNOWN, token.line, token.column, "unknown token: %.*s", 
            bohStringViewGetSize(&token.lexeme), bohStringViewGetData(&token.lexeme));

        if (token.type != BOH_TOKEN_TYPE_DUMMY) {
            bohDynArrayPushBack(&tokens, &token);
        }
    }

    return tokens;
}


bool bohLexerIsValid(bohLexer* pLexer)
{
    return pLexer && !bohStringViewIsEmpty(&pLexer->data);
}


const char* bohLexerConvertTokenTypeToStr(bohTokenType type)
{
    switch (type) {
        case BOH_TOKEN_TYPE_UNKNOWN: return "BOH_TOKEN_TYPE_UNKNOWN";
        case BOH_TOKEN_TYPE_COMMENT: return "BOH_TOKEN_TYPE_COMMENT";
        case BOH_TOKEN_TYPE_ASSIGN: return "BOH_TOKEN_TYPE_ASSIGN";
        case BOH_TOKEN_TYPE_LPAREN: return "BOH_TOKEN_TYPE_LPAREN";
        case BOH_TOKEN_TYPE_RPAREN: return "BOH_TOKEN_TYPE_RPAREN";
        case BOH_TOKEN_TYPE_LCURLY: return "BOH_TOKEN_TYPE_LCURLY";
        case BOH_TOKEN_TYPE_RCURLY: return "BOH_TOKEN_TYPE_RCURLY";
        case BOH_TOKEN_TYPE_LSQUAR: return "BOH_TOKEN_TYPE_LSQUAR";
        case BOH_TOKEN_TYPE_RSQUAR: return "BOH_TOKEN_TYPE_RSQUAR";
        case BOH_TOKEN_TYPE_COMMA: return "BOH_TOKEN_TYPE_COMMA";
        case BOH_TOKEN_TYPE_DOT: return "BOH_TOKEN_TYPE_DOT";
        case BOH_TOKEN_TYPE_PLUS: return "BOH_TOKEN_TYPE_PLUS";
        case BOH_TOKEN_TYPE_MINUS: return "BOH_TOKEN_TYPE_MINUS";
        case BOH_TOKEN_TYPE_MULT: return "BOH_TOKEN_TYPE_MULT";
        case BOH_TOKEN_TYPE_DIV: return "BOH_TOKEN_TYPE_DIV";
        case BOH_TOKEN_TYPE_MOD: return "BOH_TOKEN_TYPE_MOD";
        case BOH_TOKEN_TYPE_XOR: return "BOH_TOKEN_TYPE_XOR";
        case BOH_TOKEN_TYPE_COLON: return "BOH_TOKEN_TYPE_COLON";
        case BOH_TOKEN_TYPE_SEMICOLON: return "BOH_TOKEN_TYPE_SEMICOLON";
        case BOH_TOKEN_TYPE_QUESTION: return "BOH_TOKEN_TYPE_QUESTION";
        case BOH_TOKEN_TYPE_BITWISE_NOT: return "BOH_TOKEN_TYPE_BITWISE_NOT";
        case BOH_TOKEN_TYPE_NOT: return "BOH_TOKEN_TYPE_NOT";
        case BOH_TOKEN_TYPE_GREATER: return "BOH_TOKEN_TYPE_GREATER";
        case BOH_TOKEN_TYPE_LESS: return "BOH_TOKEN_TYPE_LESS";
        case BOH_TOKEN_TYPE_NOT_EQUAL: return "BOH_TOKEN_TYPE_NOT_EQUAL";
        case BOH_TOKEN_TYPE_GEQUAL: return "BOH_TOKEN_TYPE_GEQUAL";
        case BOH_TOKEN_TYPE_LEQUAL: return "BOH_TOKEN_TYPE_LEQUAL";
        case BOH_TOKEN_TYPE_EQUAL: return "BOH_TOKEN_TYPE_EQUAL";
        case BOH_TOKEN_TYPE_RSHIFT: return "BOH_TOKEN_TYPE_RSHIFT";
        case BOH_TOKEN_TYPE_LSHIFT: return "BOH_TOKEN_TYPE_LSHIFT";
        case BOH_TOKEN_TYPE_IDENTIFIER: return "BOH_TOKEN_TYPE_IDENTIFIER";
        case BOH_TOKEN_TYPE_STRING: return "BOH_TOKEN_TYPE_STRING";
        case BOH_TOKEN_TYPE_INTEGER: return "BOH_TOKEN_TYPE_INTEGER";
        case BOH_TOKEN_TYPE_FLOAT: return "BOH_TOKEN_TYPE_FLOAT";
        case BOH_TOKEN_TYPE_IF: return "BOH_TOKEN_TYPE_IF";
        case BOH_TOKEN_TYPE_THEN: return "BOH_TOKEN_TYPE_THEN";
        case BOH_TOKEN_TYPE_ELSE: return "BOH_TOKEN_TYPE_ELSE";
        case BOH_TOKEN_TYPE_END: return "BOH_TOKEN_TYPE_END";
        case BOH_TOKEN_TYPE_TRUE: return "BOH_TOKEN_TYPE_TRUE";
        case BOH_TOKEN_TYPE_FALSE: return "BOH_TOKEN_TYPE_FALSE";
        case BOH_TOKEN_TYPE_AND: return "BOH_TOKEN_TYPE_AND";
        case BOH_TOKEN_TYPE_OR: return "BOH_TOKEN_TYPE_OR";
        case BOH_TOKEN_TYPE_WHILE: return "BOH_TOKEN_TYPE_WHILE";
        case BOH_TOKEN_TYPE_DO: return "BOH_TOKEN_TYPE_DO";
        case BOH_TOKEN_TYPE_FOR: return "BOH_TOKEN_TYPE_FOR";
        case BOH_TOKEN_TYPE_FUNC: return "BOH_TOKEN_TYPE_FUNC";
        case BOH_TOKEN_TYPE_NULL: return "BOH_TOKEN_TYPE_NULL";
        case BOH_TOKEN_TYPE_PRINT: return "BOH_TOKEN_TYPE_PRINT";
        case BOH_TOKEN_TYPE_RETURN: return "BOH_TOKEN_TYPE_RETURN";
        default: return "INVALID";
    }
}
