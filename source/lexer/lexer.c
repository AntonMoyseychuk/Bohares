#include "pch.h"

#include "core.h"

#include "lexer.h"
#include "state.h"


#define BOH_CHECK_LEXER_COND(COND, LINE, COLUMN, FMT, ...)                  \
    if (!(COND)) {                                                          \
        char msg[1024] = {0};                                               \
        sprintf_s(msg, sizeof(msg) - 1, FMT, __VA_ARGS__);                  \
        bohStateEmplaceLexerError(bohGlobalStateGet(), (int32_t)(LINE), (int32_t)(COLUMN), msg);  \
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
    BOH_CREATE_KEY_WORD_TOKEN("else",   BOH_TOKEN_TYPE_ELSE),
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


static bool lexIsEndLineChar(char ch)
{
    return ch == '\n' || ch == '\0';
}


static bool lexIsNotEndLineChar(char ch)
{
    return !lexIsEndLineChar(ch);
}


static bool lexIsNotDoubleQuoteOrEndChar(char ch)
{
    return ch != '\"' && ch != '\0';
}


static bool lexIsNotMultilineCommentEndOrEndChar(char ch)
{
    return ch != ']' && ch != '\0';
}


static bool lexIsDigitChar(char ch)
{
    return ch >= '0' && ch <= '9';
}


static bool lexIsAlphaChar(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}


static bool lexIsUnderscoreChar(char ch)
{
    return ch == '_';
}


static bool lexIsIdentifierAppropriateChar(char ch)
{
    return lexIsAlphaChar(ch) || lexIsUnderscoreChar(ch) || lexIsDigitChar(ch);
}


static void lexProcessCarriageReturn(bohLexer* pLexer)
{
    BOH_ASSERT(pLexer);
    pLexer->column = 0;
}


static void lexProcessNewLine(bohLexer* pLexer)
{
    BOH_ASSERT(pLexer);
    
    ++pLexer->line;
    lexProcessCarriageReturn(pLexer);
}


static void lexTokenDefConstructor(void* pToken)
{
    BOH_ASSERT(pToken);
    *((bohToken*)pToken) = bohTokenCreate();
}


static void lexTokenDestructor(void* pToken)
{
    BOH_ASSERT(pToken);
    bohTokenDestroy((bohToken*)pToken);
}


static void lexTokenCopy(void* pDstToken, const void* pSrcToken)
{
    BOH_ASSERT(pDstToken);
    BOH_ASSERT(pSrcToken);
    bohTokenAssign((bohToken*)pDstToken, (const bohToken*)pSrcToken);
}


static char lexPickCurrPosChar(bohLexer* pLexer)
{
    const size_t dataSize = bohStringViewGetSize(&pLexer->data);

    return pLexer->currPos >= dataSize ? '\0' : bohStringViewAt(&pLexer->data, pLexer->currPos);
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
        if (lexIsEndLineChar(ch)) {
            lexProcessNewLine(pLexer);
        }

        lexAdvanceCurrPos(pLexer);
        ch = lexPickCurrPosChar(pLexer);
    }
    
    return ch;
}


static const bohKeyWordToken* lexConvertIdentifierLexemeToKeyWord(bohStringView tokenLexeme)
{ 
    static const bohKeyWordToken INVALID_KEY_WORD_TOKEN = BOH_CREATE_KEY_WORD_TOKEN("unknown", BOH_TOKEN_TYPE_UNKNOWN);

    for (size_t i = 0; i < BOH_KEY_WORDS_COUNT; ++i) {
        const bohKeyWordToken* pKeyWord = BOH_KEY_WORDS + i;

        if (bohStringViewEqualPtr(&pKeyWord->lexeme, &tokenLexeme)) {
            return pKeyWord;
        }
    }

    return &INVALID_KEY_WORD_TOKEN;
}


static bohStringView lexGetCurrLexem(bohLexer* pLexer)
{
    const char* pLexemBegin = bohStringViewGetData(&pLexer->data) + pLexer->startPos;
    const size_t lexemeLength = pLexer->currPos - pLexer->startPos;
    
    return bohStringViewCreateCStrSized(pLexemBegin, lexemeLength);
}


static bohToken lexGetNextToken(bohLexer* pLexer)
{
    BOH_ASSERT(pLexer);

    pLexer->startPos = pLexer->currPos;

    const bohLineNmb tokenLine = pLexer->line;
    const bohColumnNmb tokenColumn = pLexer->column;

    const char ch = lexAdvanceCurrPos(pLexer);

    bohTokenType type = BOH_TOKEN_TYPE_UNKNOWN;

    switch (ch) {
        case '\n':
            lexProcessNewLine(pLexer);
            type = BOH_TOKEN_TYPE_DUMMY;
            break;
        case '\r': 
            lexProcessCarriageReturn(pLexer);
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
        case ':': type = BOH_TOKEN_TYPE_COLON; break;
        case ';': type = BOH_TOKEN_TYPE_SEMICOLON; break;
        case '?': type = BOH_TOKEN_TYPE_QUESTION; break;
        case '+':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_PLUS_ASSIGN;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_PLUS;
                    break;
            }
            break;
        case '-':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_MINUS_ASSIGN;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_MINUS;
                    break;
            }
            break;
        case '*': 
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_MULT_ASSIGN;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_MULT;
                    break;
            }
            break;
        case '/':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_DIV_ASSIGN;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_DIV;
                    break;
            }
            break;
        case '%':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_MOD_ASSIGN;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_MOD;
                    break;
            }
            break;
        case '&':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_BITWISE_AND_ASSIGN;
                    break;
                case '&':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_AND;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_BITWISE_AND;
                    break;
            }
            break;
        case '|': 
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_BITWISE_OR_ASSIGN;
                    break;
                case '|':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_OR;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_BITWISE_OR;
                    break;
            }
            break;
        case '^':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_BITWISE_XOR_ASSIGN;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_BITWISE_XOR;
                    break;
            }
            break;
        case '~':
            switch (lexPickCurrPosChar(pLexer)) {
                case '=':
                    lexAdvanceCurrPos(pLexer);
                    type = BOH_TOKEN_TYPE_BITWISE_NOT_ASSIGN;
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_BITWISE_NOT;
                    break;
            }
            break;
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
                    switch (lexPickCurrPosChar(pLexer)) {
                        case '=':
                            lexAdvanceCurrPos(pLexer);
                            type = BOH_TOKEN_TYPE_BITWISE_RSHIFT_ASSIGN;
                            break;
                        default: 
                            type = BOH_TOKEN_TYPE_BITWISE_RSHIFT;
                            break;
                    }
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
                    switch (lexPickCurrPosChar(pLexer)) {
                        case '=':
                            lexAdvanceCurrPos(pLexer);
                            type = BOH_TOKEN_TYPE_BITWISE_LSHIFT_ASSIGN;
                            break;
                        default: 
                            type = BOH_TOKEN_TYPE_BITWISE_LSHIFT;
                            break;
                    }
                    break;
                default: 
                    type = BOH_TOKEN_TYPE_LESS;
                    break;
            }

            break;
        case '\"':
        {
            const char currCh = lexAdvanceCurrPosWhile(pLexer, lexIsNotDoubleQuoteOrEndChar);
            BOH_CHECK_LEXER_COND(currCh == '\"', pLexer->line, pLexer->column, "missed closing double quotes");

            lexAdvanceCurrPos(pLexer); // Consume '"' symbol

            type = BOH_TOKEN_TYPE_STRING;
            break;
        }
        case '#':
            if (lexPickCurrPosChar(pLexer) == '[') {
                char currCh = lexAdvanceCurrPosWhile(pLexer, lexIsNotMultilineCommentEndOrEndChar);
                BOH_CHECK_LEXER_COND(currCh == ']', pLexer->line, pLexer->column, "missed closing multiline comment symbol, expected \']#\'");

                lexAdvanceCurrPos(pLexer); // Consume ']' symbol
                
                currCh = lexPickCurrPosChar(pLexer);
                BOH_CHECK_LEXER_COND(currCh == '#', pLexer->line, pLexer->column, "missed closing multiline comment symbol, expected \']#\'");

                lexAdvanceCurrPos(pLexer); // Consume '#' symbol
            } else {
                lexAdvanceCurrPosWhile(pLexer, lexIsNotEndLineChar);
            }

            type = BOH_TOKEN_TYPE_DUMMY;
            break;
        default:
            break;
    }

    // Number
    if (type == BOH_TOKEN_TYPE_UNKNOWN && lexIsDigitChar(ch)) {
        lexAdvanceCurrPosWhile(pLexer, lexIsDigitChar);

        type = BOH_TOKEN_TYPE_INTEGER;

        if (lexPickCurrPosChar(pLexer) == '.') {
            const char nextCh = lexPickNextNStepChar(pLexer, 1);
            BOH_CHECK_LEXER_COND(lexIsDigitChar(nextCh), pLexer->line, pLexer->column, "invalid floating point number grammar");
            
            lexAdvanceCurrPos(pLexer); // Consume the '.'
                
            lexAdvanceCurrPosWhile(pLexer, lexIsDigitChar);

            type = BOH_TOKEN_TYPE_FLOAT;
        }
    }

    // Key word or identifier
    if (type == BOH_TOKEN_TYPE_UNKNOWN && (lexIsAlphaChar(ch) || lexIsUnderscoreChar(ch))) {
        lexAdvanceCurrPosWhile(pLexer, lexIsIdentifierAppropriateChar);
        
        const bohStringView lexeme = lexGetCurrLexem(pLexer);
        const bohKeyWordToken* pKeyWord = lexConvertIdentifierLexemeToKeyWord(lexeme);

        type = pKeyWord->type != BOH_TOKEN_TYPE_UNKNOWN ? pKeyWord->type : BOH_TOKEN_TYPE_IDENTIFIER;
    }

    bohStringView lexeme = lexGetCurrLexem(pLexer);
    
    // If token is string than we need to remove " symbols from final lexeme
    if (type == BOH_TOKEN_TYPE_STRING) {
        ++lexeme.pData;
        lexeme.size = lexeme.size >= 2 ? lexeme.size - 2 : 0;
    }

    return bohTokenCreateParams(lexeme, type, tokenLine, tokenColumn);
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


bohToken bohTokenCreateParams(bohStringView lexeme, bohTokenType type, bohLineNmb line, bohColumnNmb column)
{
    bohToken token;

    bohStringViewAssignStringViewPtr(&token.lexeme, &lexeme);
    token.type = type;
    token.line = line;
    token.column = column;

    return token;
}


void bohTokenDestroy(bohToken* pToken)
{
    BOH_ASSERT(pToken);

    bohStringViewReset(&pToken->lexeme);
    pToken->type = BOH_TOKEN_TYPE_UNKNOWN;
    pToken->line = 0;
    pToken->column = 0;
}


void bohTokenAssign(bohToken* pDst, const bohToken* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewAssignStringViewPtr(&pDst->lexeme, &pSrc->lexeme);
    pDst->type = pSrc->type;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;
}


const bohStringView* bohTokenGetLexeme(const bohToken* pToken)
{
    BOH_ASSERT(pToken);
    return &pToken->lexeme;
}


bohTokenType bohTokenGetType(const bohToken *pToken)
{
    BOH_ASSERT(pToken);
    return pToken->type;
}


const char* bohTokenGetTypeStr(const bohToken *pToken)
{
    BOH_ASSERT(pToken);
    return bohLexerConvertTokenTypeToStr(pToken->type);
}


bohLexer bohLexerCreate(const char* pCodeData, size_t codeDataSize)
{
    BOH_ASSERT(pCodeData);

    bohLexer lexer;

    lexer.data = bohStringViewCreateCStrSized(pCodeData, codeDataSize);

    lexer.startPos = 0;
    lexer.currPos = 0;
    
    lexer.line = 1;
    lexer.column = 0;

    lexer.tokens = BOH_DYN_ARRAY_CREATE(bohToken, lexTokenDefConstructor, lexTokenDestructor, lexTokenCopy);
    
    return lexer;
}


void bohLexerDestroy(bohLexer* pLexer)
{
    BOH_ASSERT(pLexer);

    bohStringViewReset(&pLexer->data);

    pLexer->startPos = 0;
    pLexer->currPos = 0;
    
    pLexer->line = 1;
    pLexer->column = 0;

    bohDynArrayDestroy(&pLexer->tokens);
}


const bohTokenStorage* bohLexerGetTokens(const bohLexer* pLexer)
{
    BOH_ASSERT(pLexer);
    return &pLexer->tokens;
}


void bohLexerTokenize(bohLexer* pLexer)
{
    BOH_ASSERT(pLexer);

    const size_t dataSize = bohStringViewGetSize(&pLexer->data);

    bohTokenStorage* pTokens = &pLexer->tokens;

    while (pLexer->currPos < dataSize) {
        const bohToken token = lexGetNextToken(pLexer);
        BOH_CHECK_LEXER_COND(token.type != BOH_TOKEN_TYPE_UNKNOWN, token.line, token.column, "unknown token: %.*s", 
            bohStringViewGetSize(&token.lexeme), bohStringViewGetData(&token.lexeme));

        if (token.type != BOH_TOKEN_TYPE_DUMMY) {
            bohDynArrayPushBack(pTokens, &token);
        }
    }
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
        case BOH_TOKEN_TYPE_PLUS_ASSIGN: return "BOH_TOKEN_TYPE_PLUS_ASSIGN";
        case BOH_TOKEN_TYPE_MINUS: return "BOH_TOKEN_TYPE_MINUS";
        case BOH_TOKEN_TYPE_MINUS_ASSIGN: return "BOH_TOKEN_TYPE_MINUS_ASSIGN";
        case BOH_TOKEN_TYPE_MULT: return "BOH_TOKEN_TYPE_MULT";
        case BOH_TOKEN_TYPE_MULT_ASSIGN: return "BOH_TOKEN_TYPE_MULT_ASSIGN";
        case BOH_TOKEN_TYPE_DIV: return "BOH_TOKEN_TYPE_DIV";
        case BOH_TOKEN_TYPE_DIV_ASSIGN: return "BOH_TOKEN_TYPE_DIV_ASSIGN";
        case BOH_TOKEN_TYPE_MOD: return "BOH_TOKEN_TYPE_MOD";
        case BOH_TOKEN_TYPE_MOD_ASSIGN: return "BOH_TOKEN_TYPE_MOD_ASSIGN";
        case BOH_TOKEN_TYPE_BITWISE_AND: return "BOH_TOKEN_TYPE_BITWISE_AND";
        case BOH_TOKEN_TYPE_BITWISE_AND_ASSIGN: return "BOH_TOKEN_TYPE_BITWISE_AND_ASSIGN";
        case BOH_TOKEN_TYPE_BITWISE_OR: return "BOH_TOKEN_TYPE_BITWISE_OR";
        case BOH_TOKEN_TYPE_BITWISE_OR_ASSIGN: return "BOH_TOKEN_TYPE_BITWISE_OR_ASSIGN";
        case BOH_TOKEN_TYPE_BITWISE_XOR: return "BOH_TOKEN_TYPE_BITWISE_XOR";
        case BOH_TOKEN_TYPE_BITWISE_XOR_ASSIGN: return "BOH_TOKEN_TYPE_BITWISE_XOR_ASSIGN";
        case BOH_TOKEN_TYPE_COLON: return "BOH_TOKEN_TYPE_COLON";
        case BOH_TOKEN_TYPE_SEMICOLON: return "BOH_TOKEN_TYPE_SEMICOLON";
        case BOH_TOKEN_TYPE_QUESTION: return "BOH_TOKEN_TYPE_QUESTION";
        case BOH_TOKEN_TYPE_BITWISE_NOT: return "BOH_TOKEN_TYPE_BITWISE_NOT";
        case BOH_TOKEN_TYPE_BITWISE_NOT_ASSIGN: return "BOH_TOKEN_TYPE_BITWISE_NOT_ASSIGN";
        case BOH_TOKEN_TYPE_NOT: return "BOH_TOKEN_TYPE_NOT";
        case BOH_TOKEN_TYPE_GREATER: return "BOH_TOKEN_TYPE_GREATER";
        case BOH_TOKEN_TYPE_LESS: return "BOH_TOKEN_TYPE_LESS";
        case BOH_TOKEN_TYPE_NOT_EQUAL: return "BOH_TOKEN_TYPE_NOT_EQUAL";
        case BOH_TOKEN_TYPE_GEQUAL: return "BOH_TOKEN_TYPE_GEQUAL";
        case BOH_TOKEN_TYPE_LEQUAL: return "BOH_TOKEN_TYPE_LEQUAL";
        case BOH_TOKEN_TYPE_EQUAL: return "BOH_TOKEN_TYPE_EQUAL";
        case BOH_TOKEN_TYPE_BITWISE_RSHIFT: return "BOH_TOKEN_TYPE_BITWISE_RSHIFT";
        case BOH_TOKEN_TYPE_BITWISE_RSHIFT_ASSIGN: return "BOH_TOKEN_TYPE_BITWISE_RSHIFT_ASSIGN";
        case BOH_TOKEN_TYPE_BITWISE_LSHIFT: return "BOH_TOKEN_TYPE_BITWISE_LSHIFT";
        case BOH_TOKEN_TYPE_BITWISE_LSHIFT_ASSIGN: return "BOH_TOKEN_TYPE_BITWISE_LSHIFT_ASSIGN";
        case BOH_TOKEN_TYPE_IDENTIFIER: return "BOH_TOKEN_TYPE_IDENTIFIER";
        case BOH_TOKEN_TYPE_STRING: return "BOH_TOKEN_TYPE_STRING";
        case BOH_TOKEN_TYPE_INTEGER: return "BOH_TOKEN_TYPE_INTEGER";
        case BOH_TOKEN_TYPE_FLOAT: return "BOH_TOKEN_TYPE_FLOAT";
        case BOH_TOKEN_TYPE_IF: return "BOH_TOKEN_TYPE_IF";
        case BOH_TOKEN_TYPE_ELSE: return "BOH_TOKEN_TYPE_ELSE";
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
