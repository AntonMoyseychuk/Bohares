#pragma once

#include <stdint.h>


typedef struct AST
{
    
} bohAST;


typedef struct TokenStorage bohTokenStorage;

typedef struct Parser
{
    const bohTokenStorage* pTokenStorage;
    size_t currToken;
} bohParser;


bohParser bohParserCreate(const bohTokenStorage* pTokenStorage);
void bohParserDestroy(bohParser* pParser);


bohAST bohParserParse(bohParser* pParser);
