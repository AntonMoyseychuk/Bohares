#include "pch.h"

#include "interpreter.h"

#include "model.h"
#include "parser/parser.h"


static bohNumber interpInterpretBinaryAstNode(const bohAstNodeBinary* pNode);
static bohNumber interpInterpretUnaryAstNode(const bohAstNodeUnary* pNode);


static bohNumber interpInterpretAstNode(const bohAstNode* pNode)
{
    switch (pNode->type) {
        case BOH_AST_NODE_TYPE_UNARY:   return interpInterpretUnaryAstNode(&pNode->unary);
        case BOH_AST_NODE_TYPE_BINARY:  return interpInterpretBinaryAstNode(&pNode->binary);
        default: break;
    }

    return *bohAstNodeGetNumber(pNode);
}


static bohNumber interpInterpretBinaryAstNode(const bohAstNodeBinary* pNode)
{
    const bohNumber left = interpInterpretAstNode(pNode->pLeftNode);
    const bohNumber right = interpInterpretAstNode(pNode->pRightNode);

    switch (pNode->op)
    {
        case BOH_OP_PLUS:   return bohNumberAdd(&left, &right);
        case BOH_OP_MINUS:  return bohNumberSub(&left, &right);
        case BOH_OP_MULT:   return bohNumberMult(&left, &right);
        case BOH_OP_DIV:    return bohNumberDiv(&left, &right);
    
        default:
            assert(false && "Not implemented yet");
            return bohNumberCreateI64(0);
    }
}


static bohNumber interpInterpretUnaryAstNode(const bohAstNodeUnary* pNode)
{
    const bohNumber value = interpInterpretAstNode(pNode->pNode);

    switch (pNode->op)
    {
        case BOH_OP_PLUS:           return value;
        case BOH_OP_MINUS:          return bohNumberGetOpposite(&value);
        case BOH_OP_NOT:            return bohNumberGetInverted(&value);
    
        default:
            assert(false && "Not implemented yet");
            return bohNumberCreateI64(0);
    }
}


bohInterpreter bohInterpCreate(const bohAST* pAst)
{
    assert(pAst);

    bohInterpreter interp;
    interp.pAst = pAst;

    return interp;
}


void bohInterpDestroy(bohInterpreter* pInterp)
{
    assert(pInterp);
    pInterp->pAst = NULL;
}


bohNumber bohInterpInterpret(bohInterpreter* pInterp)
{
    assert(pInterp);

    const bohAST* pAst = pInterp->pAst;
    assert(pAst);

    if (bohAstIsEmpty(pAst)) {
        return bohNumberCreateI64(0);
    }

    return interpInterpretAstNode(pAst->pRoot);
}
