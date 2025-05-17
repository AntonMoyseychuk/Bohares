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
    BOH_OP_BITWISE_LSHIFT_ASSIGN
} bohOperator;


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
};


void bohAstNodeDestroy(bohAstNode* pNode);

// Calls bohAstNodeDestroy and frees *ppNode
void bohAstNodeFree(bohAstNode** ppNode);

bohAstNode* bohAstNodeCreateNumberI64(int64_t value);
bohAstNode* bohAstNodeCreateNumberF64(double value);
bohAstNode* bohAstNodeCreateUnary(bohOperator op, bohAstNode* pArg);
bohAstNode* bohAstNodeCreateBinary(bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg);

bool bohAstNodeIsNumber(const bohAstNode* pNode);
bool bohAstNodeIsUnary(const bohAstNode* pNode);
bool bohAstNodeIsBinary(const bohAstNode* pNode);

const bohNumber* bohAstNodeGetNumber(const bohAstNode* pNode);
const bohAstNodeUnary* bohAstNodeGetUnary(const bohAstNode* pNode);
const bohAstNodeBinary* bohAstNodeGetBinary(const bohAstNode* pNode);

bohAstNode* bohAstNodeSetNumberI64(bohAstNode* pNode, int64_t value);
bohAstNode* bohAstNodeSetNumberF64(bohAstNode* pNode, double value);
bohAstNode* bohAstNodeSetUnary(bohAstNode* pNode, bohOperator op, bohAstNode* pArg);
bohAstNode* bohAstNodeSetBinary(bohAstNode* pNode, bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg);


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
