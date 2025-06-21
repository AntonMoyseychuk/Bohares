#include "pch.h"

#include "core.h"

#include "interpreter.h"
#include "parser/parser.h"

#include "types.h"
#include "error.h"


#define BOH_INTERP_PRINT_ERROR(LINE, COLUMN, FMT, ...) \
    bohErrorsStatePrintError(stderr, bohErrorsStateGerCurrProcessingFileGlobal(), LINE, COLUMN, "INTERPRETER ERROR", FMT, __VA_ARGS__)

#define BOH_INTERP_EXPECT(COND, LINE, COLUMN, FMT, ...)         \
    if (!(COND)) {                                              \
        BOH_INTERP_PRINT_ERROR(LINE, COLUMN, FMT, __VA_ARGS__); \
        bohErrorsStatePushInterpreterErrorGlobal();             \
    }


const char* bohExprInterpResultTypeToStr(bohExprInterpResultType type)
{
    switch (type) {
        case BOH_EXPR_INTERP_RES_TYPE_NUMBER: return "NUMBER";
        case BOH_EXPR_INTERP_RES_TYPE_STRING: return "STRING";
        default: return "UNKNOWN TYPE";
    }
}


bohExprInterpResult bohExprInterpResultCreate(void)
{
    bohExprInterpResult result;

    result.type = BOH_EXPR_INTERP_RES_TYPE_NUMBER;
    result.number = bohNumberCreate();

    return result;
}


void bohExprInterpResultDestroy(bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    switch (pResult->type) {
        case BOH_EXPR_INTERP_RES_TYPE_NUMBER:
            bohNumberSetI64(&pResult->number, 0);
            break;
        case BOH_EXPR_INTERP_RES_TYPE_STRING:
            bohBoharesStringDestroy(&pResult->string);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid interpretation result type");
            break;
    }

    pResult->type = BOH_EXPR_INTERP_RES_TYPE_NUMBER;
}


bohExprInterpResult* bohExprInterpResultSetStringStringViewPtr(bohExprInterpResult* pResult, const bohStringView* pStrView)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pStrView);

    bohExprInterpResultDestroy(pResult);

    pResult->type = BOH_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohExprInterpResult* bohExprInterpResultSetStringStringView(bohExprInterpResult* pResult, bohStringView strView)
{
    return bohExprInterpResultSetStringStringViewPtr(pResult, &strView);
}


bohExprInterpResult* bohExprInterpResultSetStringCStr(bohExprInterpResult* pResult, const char* pCStr)
{
    BOH_ASSERT(pCStr);
    return bohExprInterpResultSetStringStringView(pResult, bohStringViewCreateConstCStr(pCStr));
}


bohExprInterpResult* bohExprInterpResultSetString(bohExprInterpResult* pResult, const bohString* pString)
{
    BOH_ASSERT(pString);
    return bohExprInterpResultSetStringCStr(pResult, bohStringGetCStr(pString));
}


bohExprInterpResult* bohExprInterpResultSetStringViewStringViewPtr(bohExprInterpResult* pResult, const bohStringView* pStrView)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pStrView);

    bohExprInterpResultDestroy(pResult);

    pResult->type = BOH_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringViewAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohExprInterpResult* bohExprInterpResultSetStringViewStringView(bohExprInterpResult* pResult, bohStringView strView)
{
    return bohExprInterpResultSetStringViewStringViewPtr(pResult, &strView);
}


bohExprInterpResult* bohExprInterpResultSetNumberPtr(bohExprInterpResult* pResult, const bohNumber* pNumber)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pNumber);

    bohExprInterpResultDestroy(pResult);

    pResult->type = BOH_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberAssign(&pResult->number, pNumber);

    return pResult;
}


bohExprInterpResult* bohExprInterpResultSetNumber(bohExprInterpResult* pResult, bohNumber number)
{
    return bohExprInterpResultSetNumberPtr(pResult, &number);
}


bohExprInterpResult* bohExprInterpResultSetNumberI64(bohExprInterpResult* pResult, int64_t value)
{
    BOH_ASSERT(pResult);

    bohExprInterpResultDestroy(pResult);

    pResult->type = BOH_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberSetI64(&pResult->number, value);

    return pResult;
}


bohExprInterpResult* bohExprInterpResultSetNumberF64(bohExprInterpResult* pResult, double value)
{
    BOH_ASSERT(pResult);

    bohExprInterpResultDestroy(pResult);

    pResult->type = BOH_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberSetF64(&pResult->number, value);

    return pResult;
}


bohExprInterpResult bohExprInterpResultCreateString(const bohString* pString)
{
    BOH_ASSERT(pString);

    bohExprInterpResult result = bohExprInterpResultCreate();
    bohExprInterpResultSetString(&result, pString);

    return result;
}


bohExprInterpResult bohExprInterpResultCreateStringCStr(const char* pCStr)
{
    BOH_ASSERT(pCStr);

    bohExprInterpResult result = bohExprInterpResultCreate();
    bohExprInterpResultSetStringCStr(&result, pCStr);

    return result;
}


bohExprInterpResult bohExprInterpResultCreateStringStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohExprInterpResult result = bohExprInterpResultCreate();
    bohExprInterpResultSetStringStringViewPtr(&result, pStrView);

    return result;
}


bohExprInterpResult bohExprInterpResultCreateStringStringView(bohStringView strView)
{
    return bohExprInterpResultCreateStringStringViewPtr(&strView);
}


bohExprInterpResult bohExprInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    bohExprInterpResult result = bohExprInterpResultCreate();

    result.type = BOH_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringMove(&result.string, pString);

    return result;
}


bohExprInterpResult bohExprInterpResultCreateStringBoharesStringRVal(bohBoharesString string)
{
    return bohExprInterpResultCreateStringBoharesStringRValPtr(&string);
}


bohExprInterpResult bohExprInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    bohExprInterpResult result = bohExprInterpResultCreate();

    result.type = BOH_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringAssign(&result.string, pString);

    return result;
}


bohExprInterpResult bohExprInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohExprInterpResult result = bohExprInterpResultCreate();
    bohExprInterpResultSetStringViewStringViewPtr(&result, pStrView);

    return result;
}


bohExprInterpResult bohExprInterpResultCreateStringViewStringView(bohStringView strView)
{
    return bohExprInterpResultCreateStringViewStringViewPtr(&strView);
}


bohExprInterpResult bohExprInterpResultCreateNumberPtr(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);

    bohExprInterpResult result = bohExprInterpResultCreate();
    bohExprInterpResultSetNumberPtr(&result, pNumber);

    return result;
}


bohExprInterpResult bohExprInterpResultCreateNumber(bohNumber number)
{
    return bohExprInterpResultCreateNumberPtr(&number);
}


bohExprInterpResult bohExprInterpResultCreateNumberI64(int64_t value)
{
    return bohExprInterpResultCreateNumber(bohNumberCreateI64(value));
}


bohExprInterpResult bohExprInterpResultCreateNumberF64(double value)
{
    return bohExprInterpResultCreateNumber(bohNumberCreateF64(value));
}


bool bohExprInterpResultIsNumber(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_EXPR_INTERP_RES_TYPE_NUMBER;
}


bool bohExprInterpResultIsNumberI64(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohExprInterpResultIsNumber(pResult) && bohNumberIsI64(&pResult->number);
}


bool bohExprInterpResultIsNumberF64(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohExprInterpResultIsNumber(pResult) && bohNumberIsF64(&pResult->number);
}


bool bohExprInterpResultIsString(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_EXPR_INTERP_RES_TYPE_STRING;
}


bool bohExprInterpResultIsStringStringView(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohExprInterpResultIsString(pResult) && bohBoharesStringIsStringView(&pResult->string);
}


bool bohExprInterpResultIsStringString(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohExprInterpResultIsString(pResult) && bohBoharesStringIsString(&pResult->string);
}


const bohNumber* bohExprInterpResultGetNumber(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohExprInterpResultIsNumber(pResult));

    return &pResult->number;
}


int64_t bohExprInterpResultGetNumberI64(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohExprInterpResultIsNumberI64(pResult));

    return pResult->number.i64;
}


double bohExprInterpResultGetNumberF64(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohExprInterpResultIsNumberI64(pResult));

    return pResult->number.f64;
}


bool bohExprInterpResultToBool(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    if (bohExprInterpResultIsString(pResult)) {
        return true;
    }

    return bohNumberToBool(bohExprInterpResultGetNumber(pResult));
}


const bohBoharesString* bohExprInterpResultGetString(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohExprInterpResultIsString(pResult));

    return &pResult->string;
}


const bohString* bohExprInterpResultGetStringString(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohExprInterpResultIsStringString(pResult));

    return &pResult->string.string;
}


const bohStringView* bohExprInterpResultGetStringStringView(const bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohExprInterpResultIsStringStringView(pResult));

    return &pResult->string.view;
}


bohExprInterpResult* bohExprInterpResultMove(bohExprInterpResult* pDst, bohExprInterpResult* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohExprInterpResultDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_EXPR_INTERP_RES_TYPE_NUMBER:
            bohNumberMove(&pDst->number, &pSrc->number);
            break;
        case BOH_EXPR_INTERP_RES_TYPE_STRING:
            bohBoharesStringMove(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid raw expr stmt interp result type");
            break;
    }

    pSrc->type = BOH_EXPR_INTERP_RES_TYPE_NUMBER;

    return pDst;
}


bohExprInterpResult* bohExprInterpResultAssing(bohExprInterpResult* pDst, const bohExprInterpResult* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohExprInterpResultDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_EXPR_INTERP_RES_TYPE_NUMBER:
            bohNumberAssign(&pDst->number, &pSrc->number);
            break;
        case BOH_EXPR_INTERP_RES_TYPE_STRING:
            bohBoharesStringAssign(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid raw expr stmt interp result type");
            break;
    }

    return pDst;
}


bohPrintStmtInterpResult bohPrintStmtInterpResultCreate(void)
{
    bohPrintStmtInterpResult result = {0};
    return result;
}


void bohPrintStmtInterpResultDestroy(bohPrintStmtInterpResult* pResult)
{
    (void)pResult;
}


bohPrintStmtInterpResult* bohPrintStmtInterpResultMove(bohPrintStmtInterpResult* pDst, bohPrintStmtInterpResult* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    // Nothing to move

    return pDst;
}


bohPrintStmtInterpResult* bohPrintStmtInterpResultAssign(bohPrintStmtInterpResult* pDst, bohPrintStmtInterpResult* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    // Nothing to copy

    return pDst;
}


bohIfStmtInterpResult bohIfStmtInterpResultCreate(void)
{
    bohIfStmtInterpResult result = {0};
    return result;
}


void bohIfStmtInterpResultDestroy(bohIfStmtInterpResult *pResult)
{
    (void)pResult;
}


bohIfStmtInterpResult *bohIfStmtInterpResultMove(bohIfStmtInterpResult *pDst, bohIfStmtInterpResult *pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    // Nothing to move

    return pDst;
}


bohIfStmtInterpResult *bohIfStmtInterpResultAssign(bohIfStmtInterpResult *pDst, bohIfStmtInterpResult *pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    // Nothing to copy

    return pDst;
}


static bohStmtInterpResult interpCreateDummyStmtInterpResult(void)
{
    bohStmtInterpResult result = {0};
    return result;
}


static bohExprInterpResult interpInterpretBinaryExpr(const bohExpr* pExpr);
static bohExprInterpResult interpInterpretUnaryExpr(const bohExpr* pExpr);


static bohExprInterpResult interpInterpretExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    if (bohExprIsBinaryExpr(pExpr)) {
        return interpInterpretBinaryExpr(pExpr);
    } else if (bohExprIsUnaryExpr(pExpr)) {
        return interpInterpretUnaryExpr(pExpr);
    }

    if (bohExprIsValueExpr(pExpr)) {
        const bohValueExpr* pValueExpr = bohExprGetValueExpr(pExpr);

        if (bohValueExprIsNumber(pValueExpr)) {
            return bohExprInterpResultCreateNumberPtr(bohValueExprGetNumber(pValueExpr));
        } else if (bohValueExprIsString(pValueExpr)) {
            return bohExprInterpResultCreateStringBoharesStringPtr(bohValueExprGetString(pValueExpr));
        }
    }

    BOH_ASSERT_FAIL("Invalid expression type");
    return bohExprInterpResultCreateNumberI64(-1);
}


static bohExprInterpResult interpInterpretUnaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsUnaryExpr(pExpr));

    const bohUnaryExpr* pUnaryExpr = bohExprGetUnaryExpr(pExpr);

    const bohExpr* pOperandExpr = bohUnaryExprGetExpr(pUnaryExpr);
    const bohExprInterpResult result = interpInterpretExpr(pOperandExpr);
    
    const char* pOperatorStr = bohParsExprOperatorToStr(pUnaryExpr->op);
    BOH_INTERP_EXPECT(bohExprInterpResultIsNumber(&result), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "can't use unary %s operator with non numbers types", pOperatorStr);

    const bohNumber* pResultNumber = bohExprInterpResultGetNumber(&result);

    switch (pUnaryExpr->op) {
        case BOH_OP_PLUS:           return result;
        case BOH_OP_MINUS:          return bohExprInterpResultCreateNumber(bohNumberGetOpposite(pResultNumber));
        case BOH_OP_NOT:            return bohExprInterpResultCreateNumber(bohNumberGetNegation(pResultNumber));
        case BOH_OP_BITWISE_NOT:
            BOH_INTERP_EXPECT(bohNumberIsI64(pResultNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
                "can't use ~ operator with non integral type");
            return bohExprInterpResultCreateNumber(bohNumberGetBitwiseNegation(pResultNumber));
    
        default:
            BOH_ASSERT_FAIL("Invalid unary operator");
            return bohExprInterpResultCreateNumberI64(-1);
    }
}


static bohExprInterpResult interpInterpretLogicalAnd(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsBinaryExpr(pExpr));
    
    const bohBinaryExpr* pBinaryExpr = bohExprGetBinaryExpr(pExpr);

    const bohExprInterpResult leftInterpResult = interpInterpretExpr(bohBinaryExprGetLeftExpr(pBinaryExpr));

    BOH_ASSERT_MSG((bohExprInterpResultIsNumber(&leftInterpResult) || bohExprInterpResultIsString(&leftInterpResult)),
        "Invalid left bohExprInterpResult type");

    const bohNumber* pLeftNumber = bohExprInterpResultIsNumber(&leftInterpResult) ? 
        bohExprInterpResultGetNumber(&leftInterpResult) : NULL;
    
    if (pLeftNumber && bohNumberIsZero(pLeftNumber)) {
        return bohExprInterpResultCreateNumberI64(false);
    }

    const bohExprInterpResult rightInterpResult = interpInterpretExpr(bohBinaryExprGetRightExpr(pBinaryExpr));

    BOH_ASSERT_MSG((bohExprInterpResultIsNumber(&rightInterpResult) || bohExprInterpResultIsString(&rightInterpResult)),
        "Invalid right bohExprInterpResult type");

    const bohNumber* pRightNumber = bohExprInterpResultIsNumber(&rightInterpResult) ? 
        bohExprInterpResultGetNumber(&rightInterpResult) : NULL;
    
    if (pRightNumber) {
        return bohExprInterpResultCreateNumberI64(!bohNumberIsZero(pRightNumber));
    }

    return bohExprInterpResultCreateNumberI64(true);
}


static bohExprInterpResult interpInterpretLogicalOr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsBinaryExpr(pExpr));
    
    const bohBinaryExpr* pBinaryExpr = bohExprGetBinaryExpr(pExpr);

    const bohExprInterpResult leftInterpResult = interpInterpretExpr(bohBinaryExprGetLeftExpr(pBinaryExpr));

    BOH_ASSERT_MSG((bohExprInterpResultIsNumber(&leftInterpResult) || bohExprInterpResultIsString(&leftInterpResult)),
        "Invalid left bohExprInterpResult type");

    const bohNumber* pLeftNumber = bohExprInterpResultIsNumber(&leftInterpResult) ? 
        bohExprInterpResultGetNumber(&leftInterpResult) : NULL;
    
    if (pLeftNumber && !bohNumberIsZero(pLeftNumber)) {
        return bohExprInterpResultCreateNumberI64(true);
    }

    const bohExprInterpResult rightInterpResult = interpInterpretExpr(bohBinaryExprGetRightExpr(pBinaryExpr));
    
    BOH_ASSERT_MSG((bohExprInterpResultIsNumber(&rightInterpResult) || bohExprInterpResultIsString(&rightInterpResult)),
        "Invalid right bohExprInterpResult type");

    const bohNumber* pRightNumber = bohExprInterpResultIsNumber(&rightInterpResult) ? 
        bohExprInterpResultGetNumber(&rightInterpResult) : NULL;
    
    if (pRightNumber) {
        return bohExprInterpResultCreateNumberI64(!bohNumberIsZero(pRightNumber));
    }

    return bohExprInterpResultCreateNumberI64(true);
}


static bohExprInterpResult interpInterpretBinaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsBinaryExpr(pExpr));
    
    const bohBinaryExpr* pBinaryExpr = bohExprGetBinaryExpr(pExpr);

    if (pBinaryExpr->op == BOH_OP_AND) {
        return interpInterpretLogicalAnd(pExpr);
    } else if (pBinaryExpr->op == BOH_OP_OR) {
        return interpInterpretLogicalOr(pExpr);
    }

    const bohExprInterpResult left = interpInterpretExpr(bohBinaryExprGetLeftExpr(pBinaryExpr));
    const bohExprInterpResult right = interpInterpretExpr(bohBinaryExprGetRightExpr(pBinaryExpr));

    BOH_ASSERT_MSG((bohExprInterpResultIsNumber(&left) || bohExprInterpResultIsString(&left)),
        "Invalid left bohExprInterpResult type");
    BOH_ASSERT_MSG((bohExprInterpResultIsNumber(&right) || bohExprInterpResultIsString(&right)),
        "Invalid right bohExprInterpResult type");

    const bohNumber* pLeftNumber = bohExprInterpResultIsNumber(&left) ? bohExprInterpResultGetNumber(&left) : NULL;
    const bohNumber* pRightNumber = bohExprInterpResultIsNumber(&right) ? bohExprInterpResultGetNumber(&right) : NULL;

    const bohBoharesString* pLeftStr = bohExprInterpResultIsString(&left) ? bohExprInterpResultGetString(&left) : NULL;
    const bohBoharesString* pRightStr = bohExprInterpResultIsString(&right) ? bohExprInterpResultGetString(&right) : NULL;

    const char* pOperatorStr = bohParsExprOperatorToStr(pBinaryExpr->op);

    BOH_INTERP_EXPECT(left.type == right.type, bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "invalid operation: %s %s %s", 
        bohExprInterpResultTypeToStr(left.type), 
        pOperatorStr, 
        bohExprInterpResultTypeToStr(right.type));

    switch (pBinaryExpr->op) {
        case BOH_OP_PLUS:
            if (pLeftNumber) {
                return bohExprInterpResultCreateNumber(bohNumberAdd(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                const bohBoharesString finalString = bohBoharesStringAdd(pLeftStr, pRightStr);
                return bohExprInterpResultCreateStringBoharesStringPtr(&finalString);
            }
            break;
        case BOH_OP_GREATER:
            if (pLeftNumber) {
                return bohExprInterpResultCreateNumberI64(bohNumberGreater(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohExprInterpResultCreateNumberI64(bohBoharesStringGreater(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_LESS:
            if (pLeftNumber) {
                return bohExprInterpResultCreateNumberI64(bohNumberLess(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohExprInterpResultCreateNumberI64(bohBoharesStringLess(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_NOT_EQUAL:
            if (pLeftNumber) {
                return bohExprInterpResultCreateNumberI64(bohNumberNotEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohExprInterpResultCreateNumberI64(bohBoharesStringNotEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_GEQUAL:
            if (pLeftNumber) {
                return bohExprInterpResultCreateNumberI64(bohNumberGreaterEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohExprInterpResultCreateNumberI64(bohBoharesStringGreaterEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_LEQUAL:
            if (pLeftNumber) {
                return bohExprInterpResultCreateNumberI64(bohNumberLessEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohExprInterpResultCreateNumberI64(bohBoharesStringLessEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_EQUAL:
            if (pLeftNumber) {
                return bohExprInterpResultCreateNumberI64(bohNumberEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohExprInterpResultCreateNumberI64(bohBoharesStringEqual(pLeftStr, pRightStr));
            }
            break;
        default:
            break;
    }

    BOH_INTERP_EXPECT(bohExprInterpResultIsNumber(&left), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "can't use binary %s operator with non numbers types", pOperatorStr);
    BOH_INTERP_EXPECT(bohExprInterpResultIsNumber(&right), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "can't use binary %s operator with non numbers types", pOperatorStr);

    if (bohParsIsBitwiseExprOperator(pBinaryExpr->op)) {
        BOH_INTERP_EXPECT(bohNumberIsI64(pLeftNumber) && bohNumberIsI64(pRightNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
            "can't use %s bitwise operator with non integral types", pOperatorStr);
    }

    switch (pBinaryExpr->op) {
        case BOH_OP_MINUS:
            return bohExprInterpResultCreateNumber(bohNumberSub(pLeftNumber, pRightNumber));
        case BOH_OP_MULT:
            return bohExprInterpResultCreateNumber(bohNumberMult(pLeftNumber, pRightNumber));
        case BOH_OP_DIV:
            BOH_INTERP_EXPECT(!bohNumberIsZero(pRightNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), "right operand of / is zero");
            return bohExprInterpResultCreateNumber(bohNumberDiv(pLeftNumber, pRightNumber));
        case BOH_OP_MOD:
            BOH_INTERP_EXPECT(!bohNumberIsZero(pRightNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), "right operand of % is zero");
            return bohExprInterpResultCreateNumber(bohNumberMod(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_AND:
            return bohExprInterpResultCreateNumber(bohNumberBitwiseAnd(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_OR:
            return bohExprInterpResultCreateNumber(bohNumberBitwiseOr(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_XOR:
            return bohExprInterpResultCreateNumber(bohNumberBitwiseXor(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_RSHIFT:
            return bohExprInterpResultCreateNumber(bohNumberBitwiseRShift(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_LSHIFT:
            return bohExprInterpResultCreateNumber(bohNumberBitwiseLShift(pLeftNumber, pRightNumber));    
        default:
            BOH_ASSERT_FAIL("Invalid binary operator");
            return bohExprInterpResultCreateNumberI64(-1);
    }
}


static bohStmtInterpResult bohAstInterpretStmt(const bohStmt* pStmt);


static bohStmtInterpResult bohAstInterpretPrintStmt(const bohPrintStmt* pPrintStmt)
{
    BOH_ASSERT(pPrintStmt);

    bohExprInterpResult argInterpResult = interpInterpretExpr(pPrintStmt->pArgExpr);
    bohExprInterpResult* pArgInterpResult = &argInterpResult;

    if (bohExprInterpResultIsNumber(pArgInterpResult)) {
        const bohNumber* pNumber = bohExprInterpResultGetNumber(pArgInterpResult);

        if (bohNumberIsI64(pNumber)) {
            fprintf_s(stdout, "%d", bohNumberGetI64(pNumber));
        } else {
            fprintf_s(stdout, "%f", bohNumberGetF64(pNumber));
        }
    } else if (bohExprInterpResultIsString(pArgInterpResult)) {
        const bohBoharesString* pBohString = bohExprInterpResultGetString(pArgInterpResult);

        if (bohBoharesStringIsString(pBohString)) {
            const bohString* pString = bohBoharesStringGetString(pBohString);
            fprintf_s(stdout, "%s", bohStringGetCStr(pString));
        } else {
            const bohStringView* pStrView = bohBoharesStringGetStringView(pBohString);
            fprintf_s(stdout, "%.*s", bohStringViewGetSize(pStrView), bohStringViewGetData(pStrView));
        }
    } else {
        BOH_ASSERT_FAIL("Invalid raw expr stmt interp result value type");
    }

    bohExprInterpResultDestroy(pArgInterpResult);

    bohPrintStmtInterpResult interpResult = bohPrintStmtInterpResultCreate();
    return bohStmtInterpResultCreatePrintStmtMove(&interpResult);
}


static bohStmtInterpResult bohAstInterpretIfStmt(const bohIfStmt* pIfStmt)
{
    BOH_ASSERT(pIfStmt);

    bohExprInterpResult argInterpResult = interpInterpretExpr(pIfStmt->pCondExpr);
    bohExprInterpResult* pArgInterpResult = &argInterpResult;

    if (bohExprInterpResultToBool(pArgInterpResult)) {
        const size_t thenStmtCount = bohIfStmtGetThenStmtsCount(pIfStmt);

        for (size_t i = 0; i < thenStmtCount; ++i) {
            const bohStmt* pThenStmt = bohIfStmtGetThenStmtAt(pIfStmt, i);
            bohAstInterpretStmt(pThenStmt);
        }
    } else {
        const size_t elseStmtCount = bohIfStmtGetElseStmtsCount(pIfStmt);

        for (size_t i = 0; i < elseStmtCount; ++i) {
            const bohStmt* pElseStmt = bohIfStmtGetElseStmtAt(pIfStmt, i);
            bohAstInterpretStmt(pElseStmt);
        }
    }
    
    bohExprInterpResultDestroy(pArgInterpResult);

    bohIfStmtInterpResult interpResult = bohIfStmtInterpResultCreate();
    return bohStmtInterpResultCreateIfStmtMove(&interpResult);
}


static bohStmtInterpResult bohAstInterpretStmt(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);

    switch(pStmt->type) {
        case BOH_STMT_TYPE_PRINT:
            return bohAstInterpretPrintStmt(bohStmtGetPrint(pStmt));
        case BOH_STMT_TYPE_IF:
            return bohAstInterpretIfStmt(bohStmtGetIf(pStmt));
        default:
            BOH_ASSERT_FAIL("Invalid statement type");
            return interpCreateDummyStmtInterpResult();
    }
}


static void bohAstInterpretStmts(const bohAST* pAst)
{
    BOH_ASSERT(pAst);
    
    const size_t stmtCount = bohAstGetStmtCount(pAst);

    for (size_t i = 0; i < stmtCount; ++i) {
        const bohStmt* pStmt = bohAstGetStmtByIdx(pAst, i);
        bohAstInterpretStmt(pStmt);
    }
}


void bohStmtInterpResultDestroy(bohStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    switch (pResult->type) {
        case BOH_INTERP_RES_TYPE_EXPR:
            bohExprInterpResultDestroy(&pResult->exprInterpResult);
            break;
        case BOH_INTERP_RES_TYPE_PRINT:
            bohPrintStmtInterpResultDestroy(&pResult->printStmtInterpResult);
            break;
        case BOH_INTERP_RES_TYPE_IF:
            bohIfStmtInterpResultDestroy(&pResult->ifStmtInterpResult);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid stmt interpretation result type");
            break;
    }

    pResult->type = BOH_INTERP_RES_TYPE_EXPR;
}


bohStmtInterpResult bohStmtInterpResultCreateExprResultMove(bohExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    bohStmtInterpResult result = {0};
    
    result.type = BOH_INTERP_RES_TYPE_EXPR;
    bohExprInterpResultMove(&result.exprInterpResult, pResult);

    return result;
}


bohStmtInterpResult bohStmtInterpResultCreatePrintStmtMove(bohPrintStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    bohStmtInterpResult result = {0};
    
    result.type = BOH_INTERP_RES_TYPE_PRINT;
    bohPrintStmtInterpResultMove(&result.printStmtInterpResult, pResult);

    return result;
}


bohStmtInterpResult bohStmtInterpResultCreateIfStmtMove(bohIfStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    bohStmtInterpResult result = {0};
    
    result.type = BOH_INTERP_RES_TYPE_IF;
    bohIfStmtInterpResultMove(&result.ifStmtInterpResult, pResult);

    return result;
}


bool bohStmtInterpResultIsExpr(const bohStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_INTERP_RES_TYPE_EXPR;
}


bool bohStmtInterpResultIsPrintStmt(const bohStmtInterpResult *pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_INTERP_RES_TYPE_PRINT;
}


bool bohStmtInterpResultIsIfStmt(const bohStmtInterpResult *pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_INTERP_RES_TYPE_IF;
}


const bohExprInterpResult* bohStmtInterpResultGetExprResult(const bohStmtInterpResult* pResult)
{
    BOH_ASSERT(bohStmtInterpResultIsExpr(pResult));
    return &pResult->exprInterpResult;
}


const bohPrintStmtInterpResult* bohStmtInterpResultGetPrintStmtResult(const bohStmtInterpResult* pResult)
{
    BOH_ASSERT(bohStmtInterpResultIsPrintStmt(pResult));
    return &pResult->printStmtInterpResult;
}


const bohIfStmtInterpResult *bohStmtInterpResultGetIfStmtResult(const bohStmtInterpResult *pResult)
{
    BOH_ASSERT(bohStmtInterpResultIsIfStmt(pResult));
    return &pResult->ifStmtInterpResult;
}


bohInterpreter bohInterpCreate(const bohAST* pAst)
{
    BOH_ASSERT(pAst);

    bohInterpreter interp;
    interp.pAst = pAst;

    return interp;
}


void bohInterpDestroy(bohInterpreter* pInterp)
{
    BOH_ASSERT(pInterp);
    pInterp->pAst = NULL;
}


void bohInterpInterpret(bohInterpreter* pInterp)
{
    BOH_ASSERT(pInterp);
    bohAstInterpretStmts(pInterp->pAst);
}
