#include "pch.h"

#include "interpreter.h"
#include "parser/parser.h"

#include "model.h"
#include "state.h"


#define BOH_CHECK_INTERPRETER_COND(COND, LINE, COLUMN, FMT, ...)                 \
    if (!(COND)) {                                                               \
        char msg[1024] = {0};                                                    \
        sprintf_s(msg, sizeof(msg) - 1, FMT, __VA_ARGS__);                       \
        bohStateEmplaceInterpreterError(bohGlobalStateGet(), LINE, COLUMN, msg); \
        return bohNumberCreateI64(0);                                            \
    }


static bohNumber interpInterpretBinaryAstNode(const bohAstNodeBinary* pNode);
static bohNumber interpInterpretUnaryAstNode(const bohAstNodeUnary* pNode);


static bohNumber interpInterpretAstNode(const bohAstNode* pNode)
{
    assert(pNode);

    if (bohAstNodeIsBinary(pNode)) {
        return interpInterpretBinaryAstNode(bohAstNodeGetBinary(pNode));
    } else if (bohAstNodeIsUnary(pNode)) {
        return interpInterpretUnaryAstNode(bohAstNodeGetUnary(pNode));
    }

    return *bohAstNodeGetNumber(pNode);
}


static bohNumber interpInterpretBinaryAstNode(const bohAstNodeBinary* pNode)
{
    assert(pNode);

    const bohNumber left = interpInterpretAstNode(pNode->pLeftNode);
    const bohNumber right = interpInterpretAstNode(pNode->pRightNode);

    switch (pNode->op) {
        case BOH_OP_PLUS:      return bohNumberAdd(&left, &right);
        case BOH_OP_MINUS:     return bohNumberSub(&left, &right);
        case BOH_OP_MULT:      return bohNumberMult(&left, &right);
        case BOH_OP_GREATER:   return bohNumberCreateI64(bohNumberGreater(&left, &right));
        case BOH_OP_LESS:      return bohNumberCreateI64(bohNumberLess(&left, &right));
        case BOH_OP_NOT_EQUAL: return bohNumberCreateI64(bohNumberNotEqual(&left, &right));
        case BOH_OP_GEQUAL:    return bohNumberCreateI64(bohNumberGreaterEqual(&left, &right));
        case BOH_OP_LEQUAL:    return bohNumberCreateI64(bohNumberLessEqual(&left, &right));
        case BOH_OP_EQUAL:     return bohNumberCreateI64(bohNumberEqual(&left, &right));
        case BOH_OP_DIV:
            BOH_CHECK_INTERPRETER_COND(!bohNumberIsZero(&right), 0, 0, 
                "it is not possible to divide by 0"); // TODO: pass line and column inside bohAstNode
            return bohNumberDiv(&left, &right);
        case BOH_OP_MOD:
            BOH_CHECK_INTERPRETER_COND(!bohNumberIsZero(&right), 0, 0, 
                "it is not possible to take the remainder of the division by 0"); // TODO: pass line and column inside bohAstNode
            return bohNumberMod(&left, &right);
        case BOH_OP_BITWISE_AND:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use & operator with non integer types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseAnd(&left, &right);
        case BOH_OP_BITWISE_OR:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use | operator with non integer types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseOr(&left, &right);
        case BOH_OP_BITWISE_XOR:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use ^ operator with non integer types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseXor(&left, &right);
        case BOH_OP_BITWISE_RSHIFT:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use >> operator with non integer types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseRShift(&left, &right);
        case BOH_OP_BITWISE_LSHIFT:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use << operator with non integer types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseLShift(&left, &right);    
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
        case BOH_OP_NOT:            return bohNumberGetNegation(&value);
        case BOH_OP_BITWISE_NOT:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&value), 0, 0, "can't use ~ operator with non integer type");  // TODO: pass line and column inside bohAstNode
            return bohNumberGetBitwiseNegation(&value);
    
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

    return bohAstIsEmpty(pAst) ? bohNumberCreateI64(0) : interpInterpretAstNode(pAst->pRoot);
}
