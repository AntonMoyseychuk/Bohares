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


const char* bohRawExprStmtInterpResultTypeToStr(bohRawExprStmtInterpResultType type)
{
    switch (type) {
        case BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER: return "NUMBER";
        case BOH_RAW_EXPR_INTERP_RES_TYPE_STRING: return "STRING";
        default: return "UNKNOWN TYPE";
    }
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreate(void)
{
    bohRawExprStmtInterpResult result;

    result.type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    result.number = bohNumberCreate();

    return result;
}


void bohRawExprStmtInterpResultDestroy(bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    switch (pResult->type) {
        case BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER:
            bohNumberSetI64(&pResult->number, 0);
            break;
        case BOH_RAW_EXPR_INTERP_RES_TYPE_STRING:
            bohBoharesStringDestroy(&pResult->string);
            break;
        default:
            BOH_ASSERT(false && "Invalid interpretation result type");
            break;
    }

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringStringViewPtr(bohRawExprStmtInterpResult* pResult, const bohStringView* pStrView)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pStrView);

    bohRawExprStmtInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringStringView(bohRawExprStmtInterpResult* pResult, bohStringView strView)
{
    return bohRawExprStmtInterpResultSetStringStringViewPtr(pResult, &strView);
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringCStr(bohRawExprStmtInterpResult* pResult, const char* pCStr)
{
    BOH_ASSERT(pCStr);
    return bohRawExprStmtInterpResultSetStringStringView(pResult, bohStringViewCreateConstCStr(pCStr));
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetString(bohRawExprStmtInterpResult* pResult, const bohString* pString)
{
    BOH_ASSERT(pString);
    return bohRawExprStmtInterpResultSetStringCStr(pResult, bohStringGetCStr(pString));
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringViewStringViewPtr(bohRawExprStmtInterpResult* pResult, const bohStringView* pStrView)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pStrView);

    bohRawExprStmtInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringViewAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringViewStringView(bohRawExprStmtInterpResult* pResult, bohStringView strView)
{
    return bohRawExprStmtInterpResultSetStringViewStringViewPtr(pResult, &strView);
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumberPtr(bohRawExprStmtInterpResult* pResult, const bohNumber* pNumber)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pNumber);

    bohRawExprStmtInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberAssign(&pResult->number, pNumber);

    return pResult;
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumber(bohRawExprStmtInterpResult* pResult, bohNumber number)
{
    return bohRawExprStmtInterpResultSetNumberPtr(pResult, &number);
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumberI64(bohRawExprStmtInterpResult* pResult, int64_t value)
{
    BOH_ASSERT(pResult);

    bohRawExprStmtInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberSetI64(&pResult->number, value);

    return pResult;
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumberF64(bohRawExprStmtInterpResult* pResult, double value)
{
    BOH_ASSERT(pResult);

    bohRawExprStmtInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberSetF64(&pResult->number, value);

    return pResult;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateString(const bohString* pString)
{
    BOH_ASSERT(pString);

    bohRawExprStmtInterpResult result = bohRawExprStmtInterpResultCreate();
    bohRawExprStmtInterpResultSetString(&result, pString);

    return result;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringCStr(const char* pCStr)
{
    BOH_ASSERT(pCStr);

    bohRawExprStmtInterpResult result = bohRawExprStmtInterpResultCreate();
    bohRawExprStmtInterpResultSetStringCStr(&result, pCStr);

    return result;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohRawExprStmtInterpResult result = bohRawExprStmtInterpResultCreate();
    bohRawExprStmtInterpResultSetStringStringViewPtr(&result, pStrView);

    return result;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringStringView(bohStringView strView)
{
    return bohRawExprStmtInterpResultCreateStringStringViewPtr(&strView);
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    bohRawExprStmtInterpResult result = bohRawExprStmtInterpResultCreate();

    result.type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringMove(&result.string, pString);

    return result;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringBoharesStringRVal(bohBoharesString string)
{
    return bohRawExprStmtInterpResultCreateStringBoharesStringRValPtr(&string);
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    bohRawExprStmtInterpResult result = bohRawExprStmtInterpResultCreate();

    result.type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringAssign(&result.string, pString);

    return result;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohRawExprStmtInterpResult result = bohRawExprStmtInterpResultCreate();
    bohRawExprStmtInterpResultSetStringViewStringViewPtr(&result, pStrView);

    return result;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringViewStringView(bohStringView strView)
{
    return bohRawExprStmtInterpResultCreateStringViewStringViewPtr(&strView);
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumberPtr(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);

    bohRawExprStmtInterpResult result = bohRawExprStmtInterpResultCreate();
    bohRawExprStmtInterpResultSetNumberPtr(&result, pNumber);

    return result;
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumber(bohNumber number)
{
    return bohRawExprStmtInterpResultCreateNumberPtr(&number);
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumberI64(int64_t value)
{
    return bohRawExprStmtInterpResultCreateNumber(bohNumberCreateI64(value));
}


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumberF64(double value)
{
    return bohRawExprStmtInterpResultCreateNumber(bohNumberCreateF64(value));
}


bool bohRawExprStmtInterpResultIsNumber(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
}


bool bohRawExprStmtInterpResultIsNumberI64(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprStmtInterpResultIsNumber(pResult) && bohNumberIsI64(&pResult->number);
}


bool bohRawExprStmtInterpResultIsNumberF64(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprStmtInterpResultIsNumber(pResult) && bohNumberIsF64(&pResult->number);
}


bool bohRawExprStmtInterpResultIsString(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
}


bool bohRawExprStmtInterpResultIsStringStringView(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprStmtInterpResultIsString(pResult) && bohBoharesStringIsStringView(&pResult->string);
}


bool bohRawExprStmtInterpResultIsStringString(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprStmtInterpResultIsString(pResult) && bohBoharesStringIsString(&pResult->string);
}


const bohNumber* bohRawExprStmtInterpResultGetNumber(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprStmtInterpResultIsNumber(pResult));

    return &pResult->number;
}


int64_t bohRawExprStmtInterpResultGetNumberI64(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprStmtInterpResultIsNumberI64(pResult));

    return pResult->number.i64;
}


double bohRawExprStmtInterpResultGetNumberF64(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprStmtInterpResultIsNumberI64(pResult));

    return pResult->number.f64;
}


bool bohRawExprStmtInterpResultToBool(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    if (bohRawExprStmtInterpResultIsString(pResult)) {
        return true;
    }

    return bohNumberToBool(bohRawExprStmtInterpResultGetNumber(pResult));
}


const bohBoharesString* bohRawExprStmtInterpResultGetString(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprStmtInterpResultIsString(pResult));

    return &pResult->string;
}


const bohString* bohRawExprStmtInterpResultGetStringString(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprStmtInterpResultIsStringString(pResult));

    return &pResult->string.string;
}


const bohStringView* bohRawExprStmtInterpResultGetStringStringView(const bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprStmtInterpResultIsStringStringView(pResult));

    return &pResult->string.view;
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultMove(bohRawExprStmtInterpResult* pDst, bohRawExprStmtInterpResult* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohRawExprStmtInterpResultDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER:
            bohNumberMove(&pDst->number, &pSrc->number);
            break;
        case BOH_RAW_EXPR_INTERP_RES_TYPE_STRING:
            bohBoharesStringMove(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT(false && "Invalid raw expr stmt interp result type");
            break;
    }

    pSrc->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;

    return pDst;
}


bohRawExprStmtInterpResult* bohRawExprStmtInterpResultAssing(bohRawExprStmtInterpResult* pDst, const bohRawExprStmtInterpResult* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohRawExprStmtInterpResultDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER:
            bohNumberAssign(&pDst->number, &pSrc->number);
            break;
        case BOH_RAW_EXPR_INTERP_RES_TYPE_STRING:
            bohBoharesStringAssign(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT(false && "Invalid raw expr stmt interp result type");
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


static bohRawExprStmtInterpResult interpInterpretBinaryExpr(const bohExpr* pExpr);
static bohRawExprStmtInterpResult interpInterpretUnaryExpr(const bohExpr* pExpr);


static bohRawExprStmtInterpResult interpInterpretExpr(const bohExpr* pExpr)
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
            return bohRawExprStmtInterpResultCreateNumberPtr(bohValueExprGetNumber(pValueExpr));
        } else if (bohValueExprIsString(pValueExpr)) {
            return bohRawExprStmtInterpResultCreateStringBoharesStringPtr(bohValueExprGetString(pValueExpr));
        }
    }

    BOH_ASSERT(false && "Invalid expression type");
    return bohRawExprStmtInterpResultCreateNumberI64(-1);
}


static bohRawExprStmtInterpResult interpInterpretUnaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsUnaryExpr(pExpr));

    const bohUnaryExpr* pUnaryExpr = bohExprGetUnaryExpr(pExpr);

    const bohExpr* pOperandExpr = bohUnaryExprGetExpr(pUnaryExpr);
    const bohRawExprStmtInterpResult result = interpInterpretExpr(pOperandExpr);
    
    const char* pOperatorStr = bohParsExprOperatorToStr(pUnaryExpr->op);
    BOH_INTERP_EXPECT(bohRawExprStmtInterpResultIsNumber(&result), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "can't use unary %s operator with non numbers types", pOperatorStr);

    const bohNumber* pResultNumber = bohRawExprStmtInterpResultGetNumber(&result);

    switch (pUnaryExpr->op) {
        case BOH_OP_PLUS:           return result;
        case BOH_OP_MINUS:          return bohRawExprStmtInterpResultCreateNumber(bohNumberGetOpposite(pResultNumber));
        case BOH_OP_NOT:            return bohRawExprStmtInterpResultCreateNumber(bohNumberGetNegation(pResultNumber));
        case BOH_OP_BITWISE_NOT:
            BOH_INTERP_EXPECT(bohNumberIsI64(pResultNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
                "can't use ~ operator with non integral type");
            return bohRawExprStmtInterpResultCreateNumber(bohNumberGetBitwiseNegation(pResultNumber));
    
        default:
            BOH_ASSERT(false && "Invalid unary operator");
            return bohRawExprStmtInterpResultCreateNumberI64(-1);
    }
}


static bohRawExprStmtInterpResult interpInterpretLogicalAnd(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsBinaryExpr(pExpr));
    
    const bohBinaryExpr* pBinaryExpr = bohExprGetBinaryExpr(pExpr);

    const bohRawExprStmtInterpResult leftInterpResult = interpInterpretExpr(bohBinaryExprGetLeftExpr(pBinaryExpr));

    BOH_ASSERT((bohRawExprStmtInterpResultIsNumber(&leftInterpResult) || bohRawExprStmtInterpResultIsString(&leftInterpResult)) 
        && "Invalid left bohRawExprStmtInterpResult type");

    const bohNumber* pLeftNumber = bohRawExprStmtInterpResultIsNumber(&leftInterpResult) ? 
        bohRawExprStmtInterpResultGetNumber(&leftInterpResult) : NULL;
    
    if (pLeftNumber && bohNumberIsZero(pLeftNumber)) {
        return bohRawExprStmtInterpResultCreateNumberI64(false);
    }

    const bohRawExprStmtInterpResult rightInterpResult = interpInterpretExpr(bohBinaryExprGetRightExpr(pBinaryExpr));

    BOH_ASSERT((bohRawExprStmtInterpResultIsNumber(&rightInterpResult) || bohRawExprStmtInterpResultIsString(&rightInterpResult)) 
        && "Invalid right bohRawExprStmtInterpResult type");

    const bohNumber* pRightNumber = bohRawExprStmtInterpResultIsNumber(&rightInterpResult) ? 
        bohRawExprStmtInterpResultGetNumber(&rightInterpResult) : NULL;
    
    if (pRightNumber) {
        return bohRawExprStmtInterpResultCreateNumberI64(!bohNumberIsZero(pRightNumber));
    }

    return bohRawExprStmtInterpResultCreateNumberI64(true);
}


static bohRawExprStmtInterpResult interpInterpretLogicalOr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsBinaryExpr(pExpr));
    
    const bohBinaryExpr* pBinaryExpr = bohExprGetBinaryExpr(pExpr);

    const bohRawExprStmtInterpResult leftInterpResult = interpInterpretExpr(bohBinaryExprGetLeftExpr(pBinaryExpr));

    BOH_ASSERT((bohRawExprStmtInterpResultIsNumber(&leftInterpResult) || bohRawExprStmtInterpResultIsString(&leftInterpResult)) 
        && "Invalid left bohRawExprStmtInterpResult type");

    const bohNumber* pLeftNumber = bohRawExprStmtInterpResultIsNumber(&leftInterpResult) ? 
        bohRawExprStmtInterpResultGetNumber(&leftInterpResult) : NULL;
    
    if (pLeftNumber && !bohNumberIsZero(pLeftNumber)) {
        return bohRawExprStmtInterpResultCreateNumberI64(true);
    }

    const bohRawExprStmtInterpResult rightInterpResult = interpInterpretExpr(bohBinaryExprGetRightExpr(pBinaryExpr));
    
    BOH_ASSERT((bohRawExprStmtInterpResultIsNumber(&rightInterpResult) || bohRawExprStmtInterpResultIsString(&rightInterpResult))
        && "Invalid right bohRawExprStmtInterpResult type");

    const bohNumber* pRightNumber = bohRawExprStmtInterpResultIsNumber(&rightInterpResult) ? 
        bohRawExprStmtInterpResultGetNumber(&rightInterpResult) : NULL;
    
    if (pRightNumber) {
        return bohRawExprStmtInterpResultCreateNumberI64(!bohNumberIsZero(pRightNumber));
    }

    return bohRawExprStmtInterpResultCreateNumberI64(true);
}


static bohRawExprStmtInterpResult interpInterpretBinaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsBinaryExpr(pExpr));
    
    const bohBinaryExpr* pBinaryExpr = bohExprGetBinaryExpr(pExpr);

    if (pBinaryExpr->op == BOH_OP_AND) {
        return interpInterpretLogicalAnd(pExpr);
    } else if (pBinaryExpr->op == BOH_OP_OR) {
        return interpInterpretLogicalOr(pExpr);
    }

    const bohRawExprStmtInterpResult left = interpInterpretExpr(bohBinaryExprGetLeftExpr(pBinaryExpr));
    const bohRawExprStmtInterpResult right = interpInterpretExpr(bohBinaryExprGetRightExpr(pBinaryExpr));

    BOH_ASSERT((bohRawExprStmtInterpResultIsNumber(&left) || bohRawExprStmtInterpResultIsString(&left)) 
        && "Invalid left bohRawExprStmtInterpResult type");
    BOH_ASSERT((bohRawExprStmtInterpResultIsNumber(&right) || bohRawExprStmtInterpResultIsString(&right)) 
        && "Invalid right bohRawExprStmtInterpResult type");

    const bohNumber* pLeftNumber = bohRawExprStmtInterpResultIsNumber(&left) ? bohRawExprStmtInterpResultGetNumber(&left) : NULL;
    const bohNumber* pRightNumber = bohRawExprStmtInterpResultIsNumber(&right) ? bohRawExprStmtInterpResultGetNumber(&right) : NULL;

    const bohBoharesString* pLeftStr = bohRawExprStmtInterpResultIsString(&left) ? bohRawExprStmtInterpResultGetString(&left) : NULL;
    const bohBoharesString* pRightStr = bohRawExprStmtInterpResultIsString(&right) ? bohRawExprStmtInterpResultGetString(&right) : NULL;

    const char* pOperatorStr = bohParsExprOperatorToStr(pBinaryExpr->op);

    BOH_INTERP_EXPECT(left.type == right.type, bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "invalid operation: %s %s %s", 
        bohRawExprStmtInterpResultTypeToStr(left.type), 
        pOperatorStr, 
        bohRawExprStmtInterpResultTypeToStr(right.type));

    switch (pBinaryExpr->op) {
        case BOH_OP_PLUS:
            if (pLeftNumber) {
                return bohRawExprStmtInterpResultCreateNumber(bohNumberAdd(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                const bohBoharesString finalString = bohBoharesStringAdd(pLeftStr, pRightStr);
                return bohRawExprStmtInterpResultCreateStringBoharesStringPtr(&finalString);
            }
            break;
        case BOH_OP_GREATER:
            if (pLeftNumber) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohNumberGreater(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohBoharesStringGreater(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_LESS:
            if (pLeftNumber) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohNumberLess(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohBoharesStringLess(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_NOT_EQUAL:
            if (pLeftNumber) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohNumberNotEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohBoharesStringNotEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_GEQUAL:
            if (pLeftNumber) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohNumberGreaterEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohBoharesStringGreaterEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_LEQUAL:
            if (pLeftNumber) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohNumberLessEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohBoharesStringLessEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_EQUAL:
            if (pLeftNumber) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohNumberEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprStmtInterpResultCreateNumberI64(bohBoharesStringEqual(pLeftStr, pRightStr));
            }
            break;
        default:
            break;
    }

    BOH_INTERP_EXPECT(bohRawExprStmtInterpResultIsNumber(&left), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "can't use binary %s operator with non numbers types", pOperatorStr);
    BOH_INTERP_EXPECT(bohRawExprStmtInterpResultIsNumber(&right), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
        "can't use binary %s operator with non numbers types", pOperatorStr);

    if (bohParsIsBitwiseExprOperator(pBinaryExpr->op)) {
        BOH_INTERP_EXPECT(bohNumberIsI64(pLeftNumber) && bohNumberIsI64(pRightNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), 
            "can't use %s bitwise operator with non integral types", pOperatorStr);
    }

    switch (pBinaryExpr->op) {
        case BOH_OP_MINUS:
            return bohRawExprStmtInterpResultCreateNumber(bohNumberSub(pLeftNumber, pRightNumber));
        case BOH_OP_MULT:
            return bohRawExprStmtInterpResultCreateNumber(bohNumberMult(pLeftNumber, pRightNumber));
        case BOH_OP_DIV:
            BOH_INTERP_EXPECT(!bohNumberIsZero(pRightNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), "right operand of / is zero");
            return bohRawExprStmtInterpResultCreateNumber(bohNumberDiv(pLeftNumber, pRightNumber));
        case BOH_OP_MOD:
            BOH_INTERP_EXPECT(!bohNumberIsZero(pRightNumber), bohExprGetLine(pExpr), bohExprGetColumn(pExpr), "right operand of % is zero");
            return bohRawExprStmtInterpResultCreateNumber(bohNumberMod(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_AND:
            return bohRawExprStmtInterpResultCreateNumber(bohNumberBitwiseAnd(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_OR:
            return bohRawExprStmtInterpResultCreateNumber(bohNumberBitwiseOr(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_XOR:
            return bohRawExprStmtInterpResultCreateNumber(bohNumberBitwiseXor(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_RSHIFT:
            return bohRawExprStmtInterpResultCreateNumber(bohNumberBitwiseRShift(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_LSHIFT:
            return bohRawExprStmtInterpResultCreateNumber(bohNumberBitwiseLShift(pLeftNumber, pRightNumber));    
        default:
            BOH_ASSERT(false && "Invalid binary operator");
            return bohRawExprStmtInterpResultCreateNumberI64(-1);
    }
}


static bohStmtInterpResult bohAstInterpretStmt(const bohStmt* pStmt);


static bohStmtInterpResult bohAstInterpretRawExprStmt(const bohRawExprStmt* pRawExprStmt)
{
    BOH_ASSERT(pRawExprStmt);

    bohRawExprStmtInterpResult interpResult = interpInterpretExpr(pRawExprStmt->pExpr);
    return bohStmtInterpResultCreateRawExprResultMove(&interpResult);
}


static bohStmtInterpResult bohAstInterpretPrintStmt(const bohPrintStmt* pPrintStmt)
{
    BOH_ASSERT(pPrintStmt);

    bohStmtInterpResult argInterpResult = bohAstInterpretStmt(pPrintStmt->pArgStmt);

    BOH_ASSERT(bohStmtInterpResultIsRawExprStmt(&argInterpResult) && "Only raw expr for now");
    const bohRawExprStmtInterpResult* pArgRawExprStmtResult = bohStmtInterpResultGetRawExprStmtResult(&argInterpResult);

    if (bohRawExprStmtInterpResultIsNumber(pArgRawExprStmtResult)) {
        const bohNumber* pNumber = bohRawExprStmtInterpResultGetNumber(pArgRawExprStmtResult);

        if (bohNumberIsI64(pNumber)) {
            fprintf_s(stdout, "%d", bohNumberGetI64(pNumber));
        } else {
            fprintf_s(stdout, "%f", bohNumberGetF64(pNumber));
        }
    } else if (bohRawExprStmtInterpResultIsString(pArgRawExprStmtResult)) {
        const bohBoharesString* pBohString = bohRawExprStmtInterpResultGetString(pArgRawExprStmtResult);

        if (bohBoharesStringIsString(pBohString)) {
            const bohString* pString = bohBoharesStringGetString(pBohString);
            fprintf_s(stdout, "%s", bohStringGetCStr(pString));
        } else {
            const bohStringView* pStrView = bohBoharesStringGetStringView(pBohString);
            fprintf_s(stdout, "%.*s", bohStringViewGetSize(pStrView), bohStringViewGetData(pStrView));
        }
    } else {
        BOH_ASSERT(false && "Invalid raw expr stmt interp result value type");
    }

    bohStmtInterpResultDestroy(&argInterpResult);

    bohPrintStmtInterpResult interpResult = bohPrintStmtInterpResultCreate();
    return bohStmtInterpResultCreatePrintStmtMove(&interpResult);
}


static bohStmtInterpResult bohAstInterpretIfStmt(const bohIfStmt* pIfStmt)
{
    BOH_ASSERT(pIfStmt);

    bohStmtInterpResult condInterpResult = bohAstInterpretStmt(pIfStmt->pCondStmt);

    BOH_ASSERT(bohStmtInterpResultIsRawExprStmt(&condInterpResult) && "Only raw expr for now");
    const bohRawExprStmtInterpResult* pArgRawExprStmtResult = bohStmtInterpResultGetRawExprStmtResult(&condInterpResult);

    if (bohRawExprStmtInterpResultToBool(pArgRawExprStmtResult)) {
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
    
    bohStmtInterpResultDestroy(&condInterpResult);

    bohIfStmtInterpResult interpResult = bohIfStmtInterpResultCreate();
    return bohStmtInterpResultCreateIfStmtMove(&interpResult);
}


static bohStmtInterpResult bohAstInterpretStmt(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);

    switch(pStmt->type) {
        case BOH_STMT_TYPE_RAW_EXPR:
            return bohAstInterpretRawExprStmt(bohStmtGetRawExpr(pStmt));
        case BOH_STMT_TYPE_PRINT:
            return bohAstInterpretPrintStmt(bohStmtGetPrint(pStmt));
        case BOH_STMT_TYPE_IF:
            return bohAstInterpretIfStmt(bohStmtGetIf(pStmt));
        default:
            BOH_ASSERT(false && "Invalid statement type");
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
        case BOH_INTERP_RES_TYPE_RAW_EXPR:
            bohRawExprStmtInterpResultDestroy(&pResult->rawExprStmtInterpResult);
            break;
        case BOH_INTERP_RES_TYPE_PRINT:
            bohPrintStmtInterpResultDestroy(&pResult->printStmtInterpResult);
            break;
        case BOH_INTERP_RES_TYPE_IF:
            bohIfStmtInterpResultDestroy(&pResult->ifStmtInterpResult);
            break;
        default:
            BOH_ASSERT(false && "Invalid stmt interpretation result type");
            break;
    }

    pResult->type = BOH_INTERP_RES_TYPE_RAW_EXPR;
}


bohStmtInterpResult bohStmtInterpResultCreateRawExprResultMove(bohRawExprStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    bohStmtInterpResult result = {0};
    
    result.type = BOH_INTERP_RES_TYPE_RAW_EXPR;
    bohRawExprStmtInterpResultMove(&result.rawExprStmtInterpResult, pResult);

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


bool bohStmtInterpResultIsRawExprStmt(const bohStmtInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_INTERP_RES_TYPE_RAW_EXPR;
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


const bohRawExprStmtInterpResult* bohStmtInterpResultGetRawExprStmtResult(const bohStmtInterpResult* pResult)
{
    BOH_ASSERT(bohStmtInterpResultIsRawExprStmt(pResult));
    return &pResult->rawExprStmtInterpResult;
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
