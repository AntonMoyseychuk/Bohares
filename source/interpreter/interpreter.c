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
        return bohInterpResultCreateNumberI64(-1);                               \
    }


static bool bohInterpAreInterpResultValuesSameType(const bohInterpResult* pRes0, const bohInterpResult* pRes1)
{
    assert(pRes0);
    assert(pRes1);

    return bohInterpResultIsNumber(pRes0) == bohInterpResultIsNumber(pRes1) || bohInterpResultIsString(pRes0) == bohInterpResultIsString(pRes1);
}


const char* bohInterpResultTypeToStr(const bohInterpResult* pResult)
{
    assert(pResult);

    switch (pResult->type) {
        case BOH_INTERP_RES_TYPE_NUMBER: return "NUMBER";
        case BOH_INTERP_RES_TYPE_STRING: return "STRING";
        default: return "UNKNOWN TYPE";
    }
}


bohInterpResult bohInterpResultCreateString(const bohString* pString)
{
    assert(pString);

    bohInterpResult result = bohInterpResultCreate();
    bohInterpResultSetString(&result, pString);

    return result;
}


bohInterpResult bohInterpResultCreateStringCStr(const char* pCStr)
{
    assert(pCStr);

    bohInterpResult result = bohInterpResultCreate();
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

    bohInterpResult result = bohInterpResultCreate();
    bohInterpResultSetStringStringViewPtr(&result, pStrView);

    return result;
}


bohInterpResult bohInterpResultCreateStringBoharesStringRVal(bohBoharesString string)
{
    return bohInterpResultCreateStringBoharesStringRValPtr(&string);
}


bohInterpResult bohInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString)
{
    assert(pString);

    bohInterpResult result = bohInterpResultCreate();

    result.type = BOH_INTERP_RES_TYPE_STRING;
    bohBoharesStringMove(&result.string, pString);

    return result;
}


bohInterpResult bohInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString)
{
    assert(pString);

    bohInterpResult result = bohInterpResultCreate();

    result.type = BOH_INTERP_RES_TYPE_STRING;
    bohBoharesStringAssign(&result.string, pString);

    return result;
}


bohInterpResult bohInterpResultCreateStringViewStringView(bohStringView strView)
{
    return bohInterpResultCreateStringViewStringViewPtr(&strView);
}


bohInterpResult bohInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView)
{
    assert(pStrView);

    bohInterpResult result = bohInterpResultCreate();
    bohInterpResultSetStringViewStringViewPtr(&result, pStrView);

    return result;
}


bohInterpResult bohInterpResultCreate(void)
{
    bohInterpResult result;

    result.type = BOH_INTERP_RES_TYPE_NUMBER;
    result.number = bohNumberCreate();

    return result;
}


bohInterpResult bohInterpResultCreateNumberI64(int64_t value)
{
    return bohInterpResultCreateNumber(bohNumberCreateI64(value));
}


bohInterpResult bohInterpResultCreateNumberF64(double value)
{
    return bohInterpResultCreateNumber(bohNumberCreateF64(value));
}


bohInterpResult bohInterpResultCreateNumber(bohNumber number)
{
    return bohInterpResultCreateNumberPtr(&number);
}


bohInterpResult bohInterpResultCreateNumberPtr(const bohNumber* pNumber)
{
    assert(pNumber);

    bohInterpResult result = bohInterpResultCreate();
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

    memset(pResult, 0, sizeof(bohInterpResult));
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


static bohInterpResult interpInterpretBinaryAstNode(const bohAstNode* pNode);
static bohInterpResult interpInterpretUnaryAstNode(const bohAstNode* pNode);


static bohInterpResult interpInterpretAstNode(const bohAstNode* pNode)
{
    assert(pNode);

    if (bohAstNodeIsBinary(pNode)) {
        return interpInterpretBinaryAstNode(pNode);
    } else if (bohAstNodeIsUnary(pNode)) {
        return interpInterpretUnaryAstNode(pNode);
    }

    if (bohAstNodeIsNumber(pNode)) {
        return bohInterpResultCreateNumberPtr(bohAstNodeGetNumber(pNode));
    } else if (bohAstNodeIsString(pNode)) {
        return bohInterpResultCreateStringBoharesStringPtr(bohAstNodeGetString(pNode));
    }

    assert(false && "Invalid pNode type");
    return bohInterpResultCreateNumberI64(-1);
}


static bohInterpResult interpInterpretBinaryAstNode(const bohAstNode* pNode)
{
    assert(pNode);

    const bohAstNodeBinary* pBinaryNode = bohAstNodeGetBinary(pNode);

    const bohInterpResult left = interpInterpretAstNode(pBinaryNode->pLeftNode);
    const bohInterpResult right = interpInterpretAstNode(pBinaryNode->pRightNode);

    const char* pOperatorStr = bohParsOperatorToStr(pBinaryNode->op);

    BOH_CHECK_INTERPRETER_COND(bohInterpAreInterpResultValuesSameType(&left, &right), pNode->line, pNode->column, 
        "invalid operation: %s %s %s", bohInterpResultTypeToStr(&left), pOperatorStr, bohInterpResultTypeToStr(&right));

    if (pBinaryNode->op == BOH_OP_PLUS) {
        if (bohInterpResultIsNumber(&left)) {
            const bohNumber* pLeftNumber  = bohInterpResultGetNumber(&left);
            const bohNumber* pRightNumber = bohInterpResultGetNumber(&right);
            return bohInterpResultCreateNumber(bohNumberAdd(pLeftNumber, pRightNumber));
        } else {
            const bohBoharesString* pLeftStr  = bohInterpResultGetString(&left);
            const bohBoharesString* pRightStr = bohInterpResultGetString(&right);
            const bohBoharesString finalString = bohBoharesStringAdd(pLeftStr, pRightStr);
            return bohInterpResultCreateStringBoharesStringPtr(&finalString);
        }
    }

    BOH_CHECK_INTERPRETER_COND(bohInterpResultIsNumber(&left), pNode->line, pNode->column, "can't use binary %s operator with non numbers types", pOperatorStr);
    BOH_CHECK_INTERPRETER_COND(bohInterpResultIsNumber(&right), pNode->line, pNode->column, "can't use binary %s operator with non numbers types", pOperatorStr);

    const bohNumber* pLeftNumber  = bohInterpResultGetNumber(&left);
    const bohNumber* pRightNumber = bohInterpResultGetNumber(&right);

    if (bohParsIsBitwiseOperator(pBinaryNode->op)) {
        BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(pLeftNumber) && bohNumberIsI64(pRightNumber), pNode->line, pNode->column, 
            "can't use %s bitwise operator with non integral types", pOperatorStr);
    }

    assert(pLeftNumber);
    assert(pRightNumber);

    switch (pBinaryNode->op) {
        case BOH_OP_MINUS:     return bohInterpResultCreateNumber(bohNumberSub(pLeftNumber, pRightNumber));
        case BOH_OP_MULT:      return bohInterpResultCreateNumber(bohNumberMult(pLeftNumber, pRightNumber));
        case BOH_OP_GREATER:   return bohInterpResultCreateNumberI64(bohNumberGreater(pLeftNumber, pRightNumber));
        case BOH_OP_LESS:      return bohInterpResultCreateNumberI64(bohNumberLess(pLeftNumber, pRightNumber));
        case BOH_OP_NOT_EQUAL: return bohInterpResultCreateNumberI64(bohNumberNotEqual(pLeftNumber, pRightNumber));
        case BOH_OP_GEQUAL:    return bohInterpResultCreateNumberI64(bohNumberGreaterEqual(pLeftNumber, pRightNumber));
        case BOH_OP_LEQUAL:    return bohInterpResultCreateNumberI64(bohNumberLessEqual(pLeftNumber, pRightNumber));
        case BOH_OP_EQUAL:     return bohInterpResultCreateNumberI64(bohNumberEqual(pLeftNumber, pRightNumber));
        case BOH_OP_DIV:
            BOH_CHECK_INTERPRETER_COND(!bohNumberIsZero(pRightNumber), pNode->line, pNode->column, "right operand of / is zero");
            return bohInterpResultCreateNumber(bohNumberDiv(pLeftNumber, pRightNumber));
        case BOH_OP_MOD:
            BOH_CHECK_INTERPRETER_COND(!bohNumberIsZero(pRightNumber), pNode->line, pNode->column, "right operand of % is zero");
            return bohInterpResultCreateNumber(bohNumberMod(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_AND:    return bohInterpResultCreateNumber(bohNumberBitwiseAnd(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_OR:     return bohInterpResultCreateNumber(bohNumberBitwiseOr(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_XOR:    return bohInterpResultCreateNumber(bohNumberBitwiseXor(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_RSHIFT: return bohInterpResultCreateNumber(bohNumberBitwiseRShift(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_LSHIFT: return bohInterpResultCreateNumber(bohNumberBitwiseLShift(pLeftNumber, pRightNumber));    
        default:
            assert(false && "Invalid binary operator");
            return bohInterpResultCreateNumberI64(-1);
    }
}


static bohInterpResult interpInterpretUnaryAstNode(const bohAstNode* pNode)
{
    assert(pNode);

    const bohAstNodeUnary* pUnaryNode = bohAstNodeGetUnary(pNode);
    const bohInterpResult result = interpInterpretAstNode(pUnaryNode->pNode);

    BOH_CHECK_INTERPRETER_COND(bohInterpResultIsNumber(&result), pNode->line, pNode->column, "can't use unary %s operator with non numbers types", bohParsOperatorToStr(pUnaryNode->op));

    const bohNumber* pResultNumber = bohInterpResultGetNumber(&result);
    assert(pResultNumber);

    switch (pUnaryNode->op) {
        case BOH_OP_PLUS:           return result;
        case BOH_OP_MINUS:          return bohInterpResultCreateNumber(bohNumberGetOpposite(pResultNumber));
        case BOH_OP_NOT:            return bohInterpResultCreateNumber(bohNumberGetNegation(pResultNumber));
        case BOH_OP_BITWISE_NOT:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(pResultNumber), pNode->line, pNode->column, "can't use ~ operator with non integral type");
            return bohInterpResultCreateNumber(bohNumberGetBitwiseNegation(pResultNumber));
    
        default:
            assert(false && "Invalid unary operator");
            return bohInterpResultCreateNumberI64(-1);
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


bohInterpResult bohInterpInterpret(bohInterpreter* pInterp)
{
    assert(pInterp);

    const bohAST* pAst = pInterp->pAst;
    assert(pAst);

    return bohAstIsEmpty(pAst) ? bohInterpResultCreateNumberI64(0) : interpInterpretAstNode(pAst->pRoot);
}
