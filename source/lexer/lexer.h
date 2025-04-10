#pragma once

#include "utils/ds/string.h"
#include "utils/ds/string_view.h"


typedef enum TokenType
{
    TOKEN_TYPE_UNKNOWN,
    TOKEN_TYPE_COMMENT,     // #
    TOKEN_TYPE_ASSIGN,      // =
    TOKEN_TYPE_LPAREN,      // (
    TOKEN_TYPE_RPAREN,      // )
    TOKEN_TYPE_LCURLY,      // {
    TOKEN_TYPE_RCURLY,      // }
    TOKEN_TYPE_LSQUAR,      // [
    TOKEN_TYPE_RSQUAR,      // ]
    TOKEN_TYPE_COMMA,       // ,
    TOKEN_TYPE_DOT,         // .
    TOKEN_TYPE_PLUS,        // +
    TOKEN_TYPE_MINUS,       // -
    TOKEN_TYPE_MULT,        // *
    TOKEN_TYPE_DIV,         // /
    TOKEN_TYPE_MOD,         // %
    TOKEN_TYPE_CARET,       // ^
    TOKEN_TYPE_COLON,       // :
    TOKEN_TYPE_SEMICOLON,   // ;
    TOKEN_TYPE_QUESTION,    // ?
    TOKEN_TYPE_BITWISE_NOT, // ~
    TOKEN_TYPE_NOT,         // !
    TOKEN_TYPE_GREATER,     // >
    TOKEN_TYPE_LESS,        // <
    TOKEN_TYPE_NOT_EQUAL,   // !=
    TOKEN_TYPE_GEQUAL,      // >=
    TOKEN_TYPE_LEQUAL,      // <=
    TOKEN_TYPE_EQUAL,       // ==
    TOKEN_TYPE_RSHIFT,      // >>
    TOKEN_TYPE_LSHIFT,      // <<

    // Literals
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_INTEGER,
    TOKEN_TYPE_FLOAT,
    
    // Keywords
    TOKEN_TYPE_IF,
    TOKEN_TYPE_THEN,
    TOKEN_TYPE_ELSE,
    TOKEN_TYPE_END,
    TOKEN_TYPE_TRUE,
    TOKEN_TYPE_FALSE,
    TOKEN_TYPE_AND,
    TOKEN_TYPE_OR,
    TOKEN_TYPE_WHILE,
    TOKEN_TYPE_DO,
    TOKEN_TYPE_FOR,
    TOKEN_TYPE_FUNC,
    TOKEN_TYPE_NULL,
    TOKEN_TYPE_PRINT,
    TOKEN_TYPE_RETURN,

    // It is used to indicate that the token is valid, but it does not need to be added to the storage 
    // For example comments, whitespaces, new line characters, tabs, etc
    TOKEN_TYPE_DUMMY,
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

bohStringView bohTokenGetLexeme(const bohToken* pToken);
bohTokenType bohTokenGetType(const bohToken* pToken);
const char* bohTokenGetTypeStr(const bohToken* pToken);


typedef struct TokenStorage
{
    bohToken* pTokens;
    size_t size;
    size_t capacity;
} bohTokenStorage;


bohTokenStorage bohTokenStorageCreate(void);
void bohTokenStorageDestroy(bohTokenStorage* pStorage);

void bohTokenStorageReserve(bohTokenStorage* pStorage, size_t newCapacity);
void bohTokenStorageResize(bohTokenStorage* pStorage, size_t newSize);

void bohTokenStoragePushBack(bohTokenStorage* pStorage, const bohToken* pToken);

bohToken* bohTokenStorageAt(bohTokenStorage* pStorage, size_t index);

size_t bohTokenStorageGetSize(const bohTokenStorage* pStorage);
size_t bohTokenStorageGetCapacity(const bohTokenStorage* pStorage);
bool bohTokenStorageIsEmpty(const bohTokenStorage* pStorage);


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

bohTokenStorage bohLexerTokenize(bohLexer* pLexer);

bool bohLexerIsValid(bohLexer* pLexer);
