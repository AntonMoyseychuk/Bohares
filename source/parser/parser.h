#pragma once

#include "lexer/lexer.h"
#include "model.h"


typedef enum Operator
{
    BOH_OP_UNKNOWN,

    BOH_OP_PLUS,
    BOH_OP_MINUS,
    BOH_OP_MULT,
    BOH_OP_DIV,
    BOH_OP_MOD,
    BOH_OP_BITWISE_XOR,
    BOH_OP_BITWISE_NOT,
    BOH_OP_NOT,
    BOH_OP_GREATER,
    BOH_OP_LESS,
    BOH_OP_NOT_EQUAL,
    BOH_OP_GEQUAL,
    BOH_OP_LEQUAL,
    BOH_OP_EQUAL,
    BOH_OP_BITWISE_RSHIFT,
    BOH_OP_BITWISE_LSHIFT
} bohOperator;


typedef enum AstNodeType
{
    BOH_AST_NODE_TYPE_NUMBER,
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
        bohAstNodeUnary unary;
        bohAstNodeBinary binary;
        bohNumber number;
    };
};


void bohAstNodeDestroy(bohAstNode* pNode);

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

void bohAstNodeSetNumberI64(bohAstNode* pNode, int64_t value);
void bohAstNodeSetNumberF64(bohAstNode* pNode, double value);
bohAstNode* bohAstNodeSetUnary(bohAstNode* pNode, bohOperator op, bohAstNode* pArg);
bohAstNode* bohAstNodeSetBinary(bohAstNode* pNode, bohOperator op, bohAstNode* pLeftArg, bohAstNode* pRightArg);

#define BOH_AST_NODE_DESTROY(NODE_PTR) { bohAstNodeDestroy(NODE_PTR); NODE_PTR = NULL; }


typedef struct AST
{
    bohAstNode* pRoot;
} bohAST;


void bohAstDestroy(bohAST* pAST);

bool bohAstIsEmpty(const bohAST* pAST);


typedef struct Parser
{
    const bohTokenStorage* pTokenStorage;
    size_t currTokenIdx;
} bohParser;


bohParser bohParserCreate(const bohTokenStorage* pTokenStorage);
void bohParserDestroy(bohParser* pParser);


bohAST bohParserParse(bohParser* pParser);
