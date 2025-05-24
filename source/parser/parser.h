#pragma once

#include "types.h"


typedef enum Operator
{
    BOH_OP_UNKNOWN,

    BOH_OP_PLUS,
    BOH_OP_PLUS_ASSIGN,
    BOH_OP_MINUS,
    BOH_OP_MINUS_ASSIGN,
    BOH_OP_MULT,
    BOH_OP_MULT_ASSIGN,
    BOH_OP_DIV,
    BOH_OP_DIV_ASSIGN,
    BOH_OP_MOD,
    BOH_OP_MOD_ASSIGN,
    BOH_OP_NOT,
    BOH_OP_GREATER,
    BOH_OP_LESS,
    BOH_OP_NOT_EQUAL,
    BOH_OP_GEQUAL,
    BOH_OP_LEQUAL,
    BOH_OP_EQUAL,

    BOH_OP_BITWISE_AND,
    BOH_OP_BITWISE_AND_ASSIGN,
    BOH_OP_BITWISE_OR,
    BOH_OP_BITWISE_OR_ASSIGN,
    BOH_OP_BITWISE_XOR,
    BOH_OP_BITWISE_XOR_ASSIGN,
    BOH_OP_BITWISE_NOT,
    BOH_OP_BITWISE_NOT_ASSIGN,
    BOH_OP_BITWISE_RSHIFT,
    BOH_OP_BITWISE_RSHIFT_ASSIGN,
    BOH_OP_BITWISE_LSHIFT,
    BOH_OP_BITWISE_LSHIFT_ASSIGN,

    BOH_OP_AND,
    BOH_OP_OR
} bohOperator;


const char* bohParsOperatorToStr(bohOperator op);
bool bohParsIsBitwiseOperator(bohOperator op);

typedef enum AstNodeType
{
    BOH_AST_NODE_TYPE_NUMBER,
    BOH_AST_NODE_TYPE_STRING,
    BOH_AST_NODE_TYPE_UNARY,
    BOH_AST_NODE_TYPE_BINARY,
} bohAstNodeType;


typedef struct AstNode bohAstNode;


typedef struct AstNodeUnary
{
    bohOperator op;
    bohAstNode* pNode;
} bohAstNodeUnary;


typedef struct AstNodeBinary
{
    bohOperator op;
    bohAstNode* pLeftNode;
    bohAstNode* pRightNode;
} bohAstNodeBinary;


struct AstNode
{
    bohAstNodeType type;

    union 
    {
        bohNumber number;
        bohBoharesString string;
        bohAstNodeUnary unary;
        bohAstNodeBinary binary;
    };

    uint64_t line;
    uint64_t column;
};


void bohAstNodeDestroy(bohAstNode* pNode);

// Calls bohAstNodeDestroy and frees *ppNode
void bohAstNodeFree(bohAstNode** ppNode);

bohAstNode* bohAstNodeCreate(uint64_t line, uint64_t column);

bohAstNode* bohAstNodeCreateNumberI64(int64_t value, uint64_t line, uint64_t column);
bohAstNode* bohAstNodeCreateNumberF64(double value, uint64_t line, uint64_t column);
bohAstNode* bohAstNodeCreateString(const char* pCStr, uint64_t line, uint64_t column);
bohAstNode* bohAstNodeCreateStringStringView(bohStringView strView, uint64_t line, uint64_t column);
bohAstNode* bohAstNodeCreateStringStringViewPtr(const bohStringView* pStrView, uint64_t line, uint64_t column);
bohAstNode* bohAstNodeCreateUnary(bohOperator op, bohAstNode* pArg, uint64_t line, uint64_t column);
bohAstNode* bohAstNodeCreateBinary(bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg, uint64_t line, uint64_t column);

bool bohAstNodeIsNumber(const bohAstNode* pNode);
bool bohAstNodeIsNumberI64(const bohAstNode* pNode);
bool bohAstNodeIsNumberF64(const bohAstNode* pNode);
bool bohAstNodeIsString(const bohAstNode* pNode);
bool bohAstNodeIsUnary(const bohAstNode* pNode);
bool bohAstNodeIsBinary(const bohAstNode* pNode);

const bohNumber* bohAstNodeGetNumber(const bohAstNode* pNode);
int64_t bohAstNodeGetNumberI64(const bohAstNode* pNode);
double  bohAstNodeGetNumberF64(const bohAstNode* pNode);
const bohBoharesString* bohAstNodeGetString(const bohAstNode* pNode);
const bohAstNodeUnary* bohAstNodeGetUnary(const bohAstNode* pNode);
const bohAstNodeBinary* bohAstNodeGetBinary(const bohAstNode* pNode);

bohAstNode* bohAstNodeSetNumberI64(bohAstNode* pNode, int64_t value);
bohAstNode* bohAstNodeSetNumberF64(bohAstNode* pNode, double value);
bohAstNode* bohAstNodeSetStringCStr(bohAstNode* pNode, const char* pCStr);
bohAstNode* bohAstNodeSetStringString(bohAstNode* pNode, const bohString* pString);
bohAstNode* bohAstNodeSetStringStringViewPtr(bohAstNode* pNode, const bohStringView* pStrView);
bohAstNode* bohAstNodeSetUnary(bohAstNode* pNode, bohOperator op, bohAstNode* pArg);
bohAstNode* bohAstNodeSetBinary(bohAstNode* pNode, bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg);

uint64_t bohAstNodeGetLine(const bohAstNode* pNode);
uint64_t bohAstNodeGetColumn(const bohAstNode* pNode);


typedef struct AST
{
    bohAstNode* pRoot;
} bohAST;


void bohAstDestroy(bohAST* pAST);

bool bohAstIsEmpty(const bohAST* pAST);

typedef bohDynArray bohTokenStorage;

typedef struct Parser
{
    const bohTokenStorage* pTokenStorage;
    size_t currTokenIdx;
} bohParser;


bohParser bohParserCreate(const bohTokenStorage* pTokenStorage);
void bohParserDestroy(bohParser* pParser);


bohAST bohParserParse(bohParser* pParser);
