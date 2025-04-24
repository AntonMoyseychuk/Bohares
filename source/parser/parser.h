#pragma once

#include "lexer/lexer.h"


typedef enum NumberType
{
    BOH_NUMBER_TYPE_INTEGER,
    BOH_NUMBER_TYPE_FLOAT
} bohNumberType;


typedef struct Number
{
    union
    {
        int32_t i32;
        float   f32;
    };

    bohNumberType type;
    
} bohNumber;


bohNumber bohNumberCreateI32(int32_t value);
bohNumber bohNumberCreateF32(float value);

bool bohNumberIsI32(const bohNumber* pNumber);
bool bohNumberIsF32(const bohNumber* pNumber);

int32_t bohNumberGetI32(const bohNumber* pNumber);
float bohNumberGetF32(const bohNumber* pNumber);

void bohNumberSetI32(bohNumber* pNumber, int32_t value);
void bohNumberSetF32(bohNumber* pNumber, float value);


typedef struct AstNode
{

} bohAstNode;


typedef struct AST
{
    
} bohAST;


typedef struct Parser
{
    const bohTokenStorage* pTokenStorage;
    size_t currTokenIdx;
} bohParser;


bohParser bohParserCreate(const bohTokenStorage* pTokenStorage);
void bohParserDestroy(bohParser* pParser);


bohAST bohParserParse(bohParser* pParser);
