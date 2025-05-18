#include "pch.h"

#include "interpreter.h"
#include "parser/parser.h"

#include "types.h"
#include "state.h"


#define BOH_CHECK_INTERPRETER_COND(COND, LINE, COLUMN, FMT, ...)                 \
    if (!(COND)) {                                                               \
        char msg[1024] = {0};                                                    \
        sprintf_s(msg, sizeof(msg) - 1, FMT, __VA_ARGS__);                       \
        bohStateEmplaceInterpreterError(bohGlobalStateGet(), LINE, COLUMN, msg); \
        return bohNumberCreateI64(0);                                            \
    }


bohInterpResult bohInterpResultCreateString(const bohString* pString)
{
    assert(pString);

    bohInterpResult result;
    bohInterpResultSetString(&result, pString);

    return result;
}


bohInterpResult bohInterpResultCreateStringCStr(const char* pCStr)
{
    assert(pCStr);

    bohInterpResult result;
    bohInterpResultSetStringCStr(&result, pCStr);

    return result;
}


bohInterpResult bohInterpResultCreateStringStringView(bohStringView strView)
{
    return bohInterpResultCreateStringStringViewPtr(&strView);
}


bohInterpResult bohInterpResultCreateStringStringViewPtr(const bohStringView* pStrView)
{
    assert(pStrView);

    bohInterpResult result;
    bohInterpResultSetStringStringViewPtr(&result, pStrView);

    return result;
}


bohInterpResult bohInterpResultCreateStringViewStringView(bohStringView strView)
{
    return bohInterpResultCreateStringViewStringViewPtr(&strView);
}


bohInterpResult bohInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView)
{
    assert(pStrView);

    bohInterpResult result;
    bohInterpResultSetStringViewStringViewPtr(&result, pStrView);

    return result;
}


bohInterpResult bohInterpResultCreateNumber(bohNumber number)
{
    return bohInterpResultCreateNumberPtr(&number);
}


bohInterpResult bohInterpResultCreateNumberPtr(const bohNumber* pNumber)
{
    assert(pNumber);

    bohInterpResult result;
    bohInterpResultSetNumberPtr(&result, pNumber);

    return result;
}


void bohInterpResultDestroy(bohInterpResult* pResult)
{
    assert(pResult);

    switch (pResult->type) {
        case BOH_INTERP_RES_TYPE_NUMBER:
            bohNumberSetI64(&pResult->number, 0);
            break;
        case BOH_INTERP_RES_TYPE_STRING:
            bohBoharesStringDestroy(&pResult->string);
            break;
        default:
            assert(false && "Invalid interpretation result type");
            break;
    }
}


bool bohInterpResultIsNumber(const bohInterpResult* pResult)
{
    assert(pResult);
    return pResult->type == BOH_INTERP_RES_TYPE_NUMBER;
}


bool bohInterpResultIsNumberI64(const bohInterpResult* pResult)
{
    assert(pResult);
    return bohInterpResultIsNumber(pResult) && bohNumberIsI64(&pResult->number);
}


bool bohInterpResultIsNumberF64(const bohInterpResult* pResult)
{
    assert(pResult);
    return bohInterpResultIsNumber(pResult) && bohNumberIsF64(&pResult->number);
}


bool bohInterpResultIsString(const bohInterpResult* pResult)
{
    assert(pResult);
    return pResult->type == BOH_INTERP_RES_TYPE_STRING;
}


bool bohInterpResultIsStringStringView(const bohInterpResult* pResult)
{
    assert(pResult);
    return bohInterpResultIsString(pResult) && bohBoharesStringIsStringView(&pResult->string);
}


bool bohInterpResultIsStringString(const bohInterpResult* pResult)
{
    assert(pResult);
    return bohInterpResultIsString(pResult) && bohBoharesStringIsString(&pResult->string);
}


const bohNumber* bohInterpResultGetNumber(const bohInterpResult* pResult)
{
    assert(pResult);
    assert(bohInterpResultIsNumber(pResult));

    return &pResult->number;
}


int64_t bohInterpResultGetNumberI64(const bohInterpResult* pResult)
{
    assert(pResult);
    assert(bohInterpResultIsNumberI64(pResult));

    return pResult->number.i64;
}


double bohInterpResultGetNumberF64(const bohInterpResult* pResult)
{
    assert(pResult);
    assert(bohInterpResultIsNumberI64(pResult));

    return pResult->number.f64;
}


const bohBoharesString* bohInterpResultGetString(const bohInterpResult* pResult)
{
    assert(pResult);
    assert(bohInterpResultIsString(pResult));

    return &pResult->string;
}


const bohString* bohInterpResultGetStringString(const bohInterpResult* pResult)
{
    assert(pResult);
    assert(bohInterpResultIsStringString(pResult));

    return &pResult->string.string;
}


const bohStringView* bohInterpResultGetStringStringView(const bohInterpResult* pResult)
{
    assert(pResult);
    assert(bohInterpResultIsStringStringView(pResult));

    return &pResult->string.view;
}


bohInterpResult* bohInterpResultSetString(bohInterpResult* pResult, const bohString* pString)
{
    assert(pString);
    return bohInterpResultSetStringCStr(pResult, bohStringGetCStr(pString));
}


bohInterpResult* bohInterpResultSetStringCStr(bohInterpResult* pResult, const char* pCStr)
{
    assert(pCStr);
    return bohInterpResultSetStringStringView(pResult, bohStringViewCreateCStr(pCStr));
}


bohInterpResult* bohInterpResultSetStringStringView(bohInterpResult* pResult, bohStringView strView)
{
    return bohInterpResultSetStringStringViewPtr(pResult, &strView);
}


bohInterpResult* bohInterpResultSetStringStringViewPtr(bohInterpResult* pResult, const bohStringView* pStrView)
{
    assert(pResult);
    assert(pStrView);

    bohInterpResultDestroy(pResult);

    pResult->type = BOH_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohInterpResult* bohInterpResultSetStringViewStringView(bohInterpResult* pResult, bohStringView strView)
{
    return bohInterpResultSetStringViewStringViewPtr(pResult, &strView);
}


bohInterpResult* bohInterpResultSetStringViewStringViewPtr(bohInterpResult* pResult, const bohStringView* pStrView)
{
    assert(pResult);
    assert(pStrView);

    bohInterpResultDestroy(pResult);

    pResult->type = BOH_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringViewAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohInterpResult* bohInterpResultSetNumber(bohInterpResult* pResult, bohNumber number)
{
    return bohInterpResultSetNumberPtr(pResult, &number);
}


bohInterpResult* bohInterpResultSetNumberPtr(bohInterpResult* pResult, const bohNumber* pNumber)
{
    assert(pResult);
    assert(pNumber);

    bohInterpResultDestroy(pResult);

    pResult->type = BOH_INTERP_RES_TYPE_NUMBER;
    bohNumberAssign(&pResult->number, pNumber);

    return pResult;
}


bohInterpResult* bohInterpResultSetNumberI64(bohInterpResult* pResult, int64_t value)
{
    assert(pResult);

    bohInterpResultDestroy(pResult);

    pResult->type = BOH_INTERP_RES_TYPE_NUMBER;
    bohNumberSetI64(&pResult->number, value);

    return pResult;
}


bohInterpResult* bohInterpResultSetNumberF64(bohInterpResult* pResult, double value)
{
    assert(pResult);

    bohInterpResultDestroy(pResult);

    pResult->type = BOH_INTERP_RES_TYPE_NUMBER;
    bohNumberSetF64(&pResult->number, value);

    return pResult;
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
                "right operand of / is zero"); // TODO: pass line and column inside bohAstNode
            return bohNumberDiv(&left, &right);
        case BOH_OP_MOD:
            BOH_CHECK_INTERPRETER_COND(!bohNumberIsZero(&right), 0, 0, 
                "right operand of % is zero"); // TODO: pass line and column inside bohAstNode
            return bohNumberMod(&left, &right);
        case BOH_OP_BITWISE_AND:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use & operator with non integral types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseAnd(&left, &right);
        case BOH_OP_BITWISE_OR:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use | operator with non integral types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseOr(&left, &right);
        case BOH_OP_BITWISE_XOR:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use ^ operator with non integral types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseXor(&left, &right);
        case BOH_OP_BITWISE_RSHIFT:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use >> operator with non integral types"); // TODO: pass line and column inside bohAstNode
            return bohNumberBitwiseRShift(&left, &right);
        case BOH_OP_BITWISE_LSHIFT:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&left) && bohNumberIsI64(&right), 0, 0, 
                "can't use << operator with non integral types"); // TODO: pass line and column inside bohAstNode
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
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(&value), 0, 0, "can't use ~ operator with non integral type");  // TODO: pass line and column inside bohAstNode
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
