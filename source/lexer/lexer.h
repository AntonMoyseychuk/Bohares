#pragma once

#include "utils/ds/string.h"
#include "utils/ds/string_view.h"
#include "utils/ds/dyn_array.h"


typedef enum TokenType
{
    BOH_TOKEN_TYPE_UNKNOWN,
    BOH_TOKEN_TYPE_COMMENT,     // #
    BOH_TOKEN_TYPE_ASSIGN,      // =
    BOH_TOKEN_TYPE_LPAREN,      // (
    BOH_TOKEN_TYPE_RPAREN,      // )
    BOH_TOKEN_TYPE_LCURLY,      // {
    BOH_TOKEN_TYPE_RCURLY,      // }
    BOH_TOKEN_TYPE_LSQUAR,      // [
    BOH_TOKEN_TYPE_RSQUAR,      // ]
    BOH_TOKEN_TYPE_COMMA,       // ,
    BOH_TOKEN_TYPE_DOT,         // .
    BOH_TOKEN_TYPE_PLUS,        // +
    BOH_TOKEN_TYPE_MINUS,       // -
    BOH_TOKEN_TYPE_MULT,        // *
    BOH_TOKEN_TYPE_DIV,         // /
    BOH_TOKEN_TYPE_MOD,         // %
    BOH_TOKEN_TYPE_COLON,       // :
    BOH_TOKEN_TYPE_SEMICOLON,   // ;
    BOH_TOKEN_TYPE_QUESTION,    // ?
    BOH_TOKEN_TYPE_NOT,         // !
    BOH_TOKEN_TYPE_GREATER,     // >
    BOH_TOKEN_TYPE_LESS,        // <
    BOH_TOKEN_TYPE_NOT_EQUAL,   // !=
    BOH_TOKEN_TYPE_GEQUAL,      // >=
    BOH_TOKEN_TYPE_LEQUAL,      // <=
    BOH_TOKEN_TYPE_EQUAL,       // ==
    BOH_TOKEN_TYPE_BITWISE_XOR, // ^
    BOH_TOKEN_TYPE_BITWISE_NOT, // ~
    BOH_TOKEN_TYPE_BITWISE_RSHIFT, // >>
    BOH_TOKEN_TYPE_BITWISE_LSHIFT, // <<

    // Literals
    BOH_TOKEN_TYPE_IDENTIFIER,
    BOH_TOKEN_TYPE_STRING,
    BOH_TOKEN_TYPE_INTEGER,
    BOH_TOKEN_TYPE_FLOAT,
    
    // Keywords
    BOH_TOKEN_TYPE_IF,
    BOH_TOKEN_TYPE_ELSE,
    BOH_TOKEN_TYPE_TRUE,
    BOH_TOKEN_TYPE_FALSE,
    BOH_TOKEN_TYPE_AND,
    BOH_TOKEN_TYPE_OR,
    BOH_TOKEN_TYPE_WHILE,
    BOH_TOKEN_TYPE_DO,
    BOH_TOKEN_TYPE_FOR,
    BOH_TOKEN_TYPE_FUNC,
    BOH_TOKEN_TYPE_NULL,
    BOH_TOKEN_TYPE_PRINT,
    BOH_TOKEN_TYPE_RETURN,

    // It is used to indicate that the token is valid, but it does not need to be added to the storage 
    // For example comments, whitespaces, new line characters, tabs, etc
    BOH_TOKEN_TYPE_DUMMY,
} bohTokenType;


typedef struct Token
{
    bohStringView lexeme;
    bohTokenType type;
    uint32_t line;
    uint32_t column;
} bohToken;


bohToken bohTokenCreate(void);
bohToken bohTokenCreateParams(bohStringView lexeme, bohTokenType type, uint32_t line, uint32_t column);

void bohTokenDestroy(bohToken* pToken);

void bohTokenAssign(bohToken* pDst, const bohToken* pSrc);

const bohStringView* bohTokenGetLexeme(const bohToken* pToken);
bohTokenType bohTokenGetType(const bohToken* pToken);
const char* bohTokenGetTypeStr(const bohToken* pToken);


typedef struct Lexer
{
    bohStringView data;

    size_t startPos; // Begin of current lexeme
    size_t currPos;  // Current position inside current lexeme

    size_t line;     // Lexeme line
    size_t column;   // Lexeme column
} bohLexer;


bohLexer bohLexerCreate(const char* pCodeData, size_t codeDataSize);
void bohLexerDestroy(bohLexer* pLexer);

typedef bohDynArray bohTokenStorage;
bohTokenStorage bohLexerTokenize(bohLexer* pLexer);

bool bohLexerIsValid(bohLexer* pLexer);


const char* bohLexerConvertTokenTypeToStr(bohTokenType type);
