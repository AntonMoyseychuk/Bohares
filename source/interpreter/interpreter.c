#include "pch.h"

#include "core.h"

#include "interpreter.h"
#include "parser/parser.h"

#include "types.h"
#include "state.h"


#define BOH_CHECK_INTERPRETER_COND(COND, LINE, COLUMN, FMT, ...)                 \
    if (!(COND)) {                                                               \
        char msg[1024] = {0};                                                    \
        sprintf_s(msg, sizeof(msg) - 1, FMT, __VA_ARGS__);                       \
        bohStateEmplaceInterpreterError(bohGlobalStateGet(), LINE, COLUMN, msg); \
        return bohRawExprInterpResultCreateNumberI64(-1);                        \
    }


typedef enum RawExprInterpResultType
{
    BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER,
    BOH_RAW_EXPR_INTERP_RES_TYPE_STRING
} bohRawExprInterpResultType;


typedef struct RawExprInterpResult
{
    bohRawExprInterpResultType type;
    
    union {
        bohBoharesString string;
        bohNumber number;
    };
} bohRawExprInterpResult;


const char* bohRawExprInterpResultTypeToStr(const bohRawExprInterpResult* pResult);


bohRawExprInterpResult bohInterpResultCreate(void);

bohRawExprInterpResult bohRawExprInterpResultCreateNumberI64(int64_t value);
bohRawExprInterpResult bohRawExprInterpResultCreateNumberF64(double value);
bohRawExprInterpResult bohRawExprInterpResultCreateNumber(bohNumber number);
bohRawExprInterpResult bohRawExprInterpResultCreateNumberPtr(const bohNumber* pNumber);

bohRawExprInterpResult bohRawExprInterpResultCreateString(const bohString* pString);
bohRawExprInterpResult bohRawExprInterpResultCreateStringCStr(const char* pCStr);
bohRawExprInterpResult bohRawExprInterpResultCreateStringStringView(bohStringView strView);
bohRawExprInterpResult bohRawExprInterpResultCreateStringStringViewPtr(const bohStringView* pStrView);
bohRawExprInterpResult bohRawExprInterpResultCreateStringBoharesStringRVal(bohBoharesString string);
bohRawExprInterpResult bohRawExprInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString);
bohRawExprInterpResult bohRawExprInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString);

bohRawExprInterpResult bohRawExprInterpResultCreateStringViewStringView(bohStringView strView);
bohRawExprInterpResult bohRawExprInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView);

void bohRawExprInterpResultDestroy(bohRawExprInterpResult* pResult);

bool bohRawExprInterpResultIsNumber(const bohRawExprInterpResult* pResult);
bool bohRawExprInterpResultIsNumberI64(const bohRawExprInterpResult* pResult);
bool bohRawExprInterpResultIsNumberF64(const bohRawExprInterpResult* pResult);
bool bohRawExprInterpResultIsString(const bohRawExprInterpResult* pResult);
bool bohRawExprInterpResultIsStringStringView(const bohRawExprInterpResult* pResult);
bool bohRawExprInterpResultIsStringString(const bohRawExprInterpResult* pResult);

const bohNumber*        bohRawExprInterpResultGetNumber(const bohRawExprInterpResult* pResult);
int64_t                 bohRawExprInterpResultGetNumberI64(const bohRawExprInterpResult* pResult);
double                  bohRawExprInterpResultGetNumberF64(const bohRawExprInterpResult* pResult);
const bohBoharesString* bohRawExprInterpResultGetString(const bohRawExprInterpResult* pResult);
const bohString*        bohRawExprInterpResultGetStringString(const bohRawExprInterpResult* pResult);
const bohStringView*    bohRawExprInterpResultGetStringStringView(const bohRawExprInterpResult* pResult);

bohRawExprInterpResult* bohRawExprInterpResultSetString(bohRawExprInterpResult* pResult, const bohString* pString);
bohRawExprInterpResult* bohRawExprInterpResultSetStringCStr(bohRawExprInterpResult* pResult, const char* pCStr);
bohRawExprInterpResult* bohRawExprInterpResultSetStringStringView(bohRawExprInterpResult* pResult, bohStringView strView);
bohRawExprInterpResult* bohRawExprInterpResultSetStringStringViewPtr(bohRawExprInterpResult* pResult, const bohStringView* pStrView);
bohRawExprInterpResult* bohRawExprInterpResultSetStringViewStringView(bohRawExprInterpResult* pResult, bohStringView strView);
bohRawExprInterpResult* bohRawExprInterpResultSetStringViewStringViewPtr(bohRawExprInterpResult* pResult, const bohStringView* pStrView);
bohRawExprInterpResult* bohRawExprInterpResultSetNumber(bohRawExprInterpResult* pResult, bohNumber number);
bohRawExprInterpResult* bohRawExprInterpResultSetNumberPtr(bohRawExprInterpResult* pResult, const bohNumber* pNumber);
bohRawExprInterpResult* bohRawExprInterpResultSetNumberI64(bohRawExprInterpResult* pResult, int64_t value);
bohRawExprInterpResult* bohRawExprInterpResultSetNumberF64(bohRawExprInterpResult* pResult, double value);


const char* bohRawExprInterpResultTypeToStr(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    switch (pResult->type) {
        case BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER: return "NUMBER";
        case BOH_RAW_EXPR_INTERP_RES_TYPE_STRING: return "STRING";
        default: return "UNKNOWN TYPE";
    }
}


bohRawExprInterpResult bohRawExprInterpResultCreateString(const bohString* pString)
{
    BOH_ASSERT(pString);

    bohRawExprInterpResult result = bohInterpResultCreate();
    bohRawExprInterpResultSetString(&result, pString);

    return result;
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringCStr(const char* pCStr)
{
    BOH_ASSERT(pCStr);

    bohRawExprInterpResult result = bohInterpResultCreate();
    bohRawExprInterpResultSetStringCStr(&result, pCStr);

    return result;
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringStringView(bohStringView strView)
{
    return bohRawExprInterpResultCreateStringStringViewPtr(&strView);
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohRawExprInterpResult result = bohInterpResultCreate();
    bohRawExprInterpResultSetStringStringViewPtr(&result, pStrView);

    return result;
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringBoharesStringRVal(bohBoharesString string)
{
    return bohRawExprInterpResultCreateStringBoharesStringRValPtr(&string);
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    bohRawExprInterpResult result = bohInterpResultCreate();

    result.type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringMove(&result.string, pString);

    return result;
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    bohRawExprInterpResult result = bohInterpResultCreate();

    result.type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringAssign(&result.string, pString);

    return result;
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringViewStringView(bohStringView strView)
{
    return bohRawExprInterpResultCreateStringViewStringViewPtr(&strView);
}


bohRawExprInterpResult bohRawExprInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohRawExprInterpResult result = bohInterpResultCreate();
    bohRawExprInterpResultSetStringViewStringViewPtr(&result, pStrView);

    return result;
}


bohRawExprInterpResult bohInterpResultCreate(void)
{
    bohRawExprInterpResult result;

    result.type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    result.number = bohNumberCreate();

    return result;
}


bohRawExprInterpResult bohRawExprInterpResultCreateNumberI64(int64_t value)
{
    return bohRawExprInterpResultCreateNumber(bohNumberCreateI64(value));
}


bohRawExprInterpResult bohRawExprInterpResultCreateNumberF64(double value)
{
    return bohRawExprInterpResultCreateNumber(bohNumberCreateF64(value));
}


bohRawExprInterpResult bohRawExprInterpResultCreateNumber(bohNumber number)
{
    return bohRawExprInterpResultCreateNumberPtr(&number);
}


bohRawExprInterpResult bohRawExprInterpResultCreateNumberPtr(const bohNumber* pNumber)
{
    BOH_ASSERT(pNumber);

    bohRawExprInterpResult result = bohInterpResultCreate();
    bohRawExprInterpResultSetNumberPtr(&result, pNumber);

    return result;
}


void bohRawExprInterpResultDestroy(bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;

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
}


bool bohRawExprInterpResultIsNumber(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
}


bool bohRawExprInterpResultIsNumberI64(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprInterpResultIsNumber(pResult) && bohNumberIsI64(&pResult->number);
}


bool bohRawExprInterpResultIsNumberF64(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprInterpResultIsNumber(pResult) && bohNumberIsF64(&pResult->number);
}


bool bohRawExprInterpResultIsString(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return pResult->type == BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
}


bool bohRawExprInterpResultIsStringStringView(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprInterpResultIsString(pResult) && bohBoharesStringIsStringView(&pResult->string);
}


bool bohRawExprInterpResultIsStringString(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    return bohRawExprInterpResultIsString(pResult) && bohBoharesStringIsString(&pResult->string);
}


const bohNumber* bohRawExprInterpResultGetNumber(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprInterpResultIsNumber(pResult));

    return &pResult->number;
}


int64_t bohRawExprInterpResultGetNumberI64(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprInterpResultIsNumberI64(pResult));

    return pResult->number.i64;
}


double bohRawExprInterpResultGetNumberF64(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprInterpResultIsNumberI64(pResult));

    return pResult->number.f64;
}


const bohBoharesString* bohRawExprInterpResultGetString(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprInterpResultIsString(pResult));

    return &pResult->string;
}


const bohString* bohRawExprInterpResultGetStringString(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprInterpResultIsStringString(pResult));

    return &pResult->string.string;
}


const bohStringView* bohRawExprInterpResultGetStringStringView(const bohRawExprInterpResult* pResult)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(bohRawExprInterpResultIsStringStringView(pResult));

    return &pResult->string.view;
}


bohRawExprInterpResult* bohRawExprInterpResultSetString(bohRawExprInterpResult* pResult, const bohString* pString)
{
    BOH_ASSERT(pString);
    return bohRawExprInterpResultSetStringCStr(pResult, bohStringGetCStr(pString));
}


bohRawExprInterpResult* bohRawExprInterpResultSetStringCStr(bohRawExprInterpResult* pResult, const char* pCStr)
{
    BOH_ASSERT(pCStr);
    return bohRawExprInterpResultSetStringStringView(pResult, bohStringViewCreateCStr(pCStr));
}


bohRawExprInterpResult* bohRawExprInterpResultSetStringStringView(bohRawExprInterpResult* pResult, bohStringView strView)
{
    return bohRawExprInterpResultSetStringStringViewPtr(pResult, &strView);
}


bohRawExprInterpResult* bohRawExprInterpResultSetStringStringViewPtr(bohRawExprInterpResult* pResult, const bohStringView* pStrView)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pStrView);

    bohRawExprInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohRawExprInterpResult* bohRawExprInterpResultSetStringViewStringView(bohRawExprInterpResult* pResult, bohStringView strView)
{
    return bohRawExprInterpResultSetStringViewStringViewPtr(pResult, &strView);
}


bohRawExprInterpResult* bohRawExprInterpResultSetStringViewStringViewPtr(bohRawExprInterpResult* pResult, const bohStringView* pStrView)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pStrView);

    bohRawExprInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_STRING;
    bohBoharesStringStringViewAssignStringViewPtr(&pResult->string, pStrView);

    return pResult;
}


bohRawExprInterpResult* bohRawExprInterpResultSetNumber(bohRawExprInterpResult* pResult, bohNumber number)
{
    return bohRawExprInterpResultSetNumberPtr(pResult, &number);
}


bohRawExprInterpResult* bohRawExprInterpResultSetNumberPtr(bohRawExprInterpResult* pResult, const bohNumber* pNumber)
{
    BOH_ASSERT(pResult);
    BOH_ASSERT(pNumber);

    bohRawExprInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberAssign(&pResult->number, pNumber);

    return pResult;
}


bohRawExprInterpResult* bohRawExprInterpResultSetNumberI64(bohRawExprInterpResult* pResult, int64_t value)
{
    BOH_ASSERT(pResult);

    bohRawExprInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberSetI64(&pResult->number, value);

    return pResult;
}


bohRawExprInterpResult* bohRawExprInterpResultSetNumberF64(bohRawExprInterpResult* pResult, double value)
{
    BOH_ASSERT(pResult);

    bohRawExprInterpResultDestroy(pResult);

    pResult->type = BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER;
    bohNumberSetF64(&pResult->number, value);

    return pResult;
}

#if 0

static bohRawExprInterpResult interpInterpretBinaryAstNode(const bohAstNode* pNode);
static bohRawExprInterpResult interpInterpretUnaryAstNode(const bohAstNode* pNode);


static bohRawExprInterpResult interpInterpretAstNode(const bohAstNode* pNode)
{
    BOH_ASSERT(pNode);

    if (bohAstNodeIsBinary(pNode)) {
        return interpInterpretBinaryAstNode(pNode);
    } else if (bohAstNodeIsUnary(pNode)) {
        return interpInterpretUnaryAstNode(pNode);
    }

    if (bohAstNodeIsNumber(pNode)) {
        return bohRawExprInterpResultCreateNumberPtr(bohAstNodeGetNumber(pNode));
    } else if (bohAstNodeIsString(pNode)) {
        return bohRawExprInterpResultCreateStringBoharesStringPtr(bohAstNodeGetString(pNode));
    }

    BOH_ASSERT(false && "Invalid pNode type");
    return bohRawExprInterpResultCreateNumberI64(-1);
}


static bohRawExprInterpResult interpInterpretUnaryAstNode(const bohAstNode* pNode)
{
    BOH_ASSERT(pNode);

    const bohAstNodeUnary* pUnaryNode = bohAstNodeGetUnary(pNode);

    const bohRawExprInterpResult result = interpInterpretAstNode(pUnaryNode->pNode);
    
    const char* pOperatorStr = bohParsExprOperatorToStr(pUnaryNode->op);
    BOH_CHECK_INTERPRETER_COND(bohRawExprInterpResultIsNumber(&result), pNode->line, pNode->column, 
        "can't use unary %s operator with non numbers types", pOperatorStr);

    const bohNumber* pResultNumber = bohRawExprInterpResultGetNumber(&result);
    BOH_ASSERT(pResultNumber);

    switch (pUnaryNode->op) {
        case BOH_OP_PLUS:           return result;
        case BOH_OP_MINUS:          return bohRawExprInterpResultCreateNumber(bohNumberGetOpposite(pResultNumber));
        case BOH_OP_NOT:            return bohRawExprInterpResultCreateNumber(bohNumberGetNegation(pResultNumber));
        case BOH_OP_BITWISE_NOT:
            BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(pResultNumber), pNode->line, pNode->column, "can't use ~ operator with non integral type");
            return bohRawExprInterpResultCreateNumber(bohNumberGetBitwiseNegation(pResultNumber));
    
        default:
            BOH_ASSERT(false && "Invalid unary operator");
            return bohRawExprInterpResultCreateNumberI64(-1);
    }
}


static bohRawExprInterpResult interpInterpretLogicalAnd(const bohAstNodeBinary* pBinaryNode)
{
    BOH_ASSERT(pBinaryNode);
    
    const bohRawExprInterpResult leftInterpResult = interpInterpretAstNode(pBinaryNode->pLeftNode);
    BOH_ASSERT((bohRawExprInterpResultIsNumber(&leftInterpResult) || bohRawExprInterpResultIsString(&leftInterpResult)) && "Invalid left bohRawExprInterpResult type");

    const bohNumber* pLeftNumber = bohRawExprInterpResultIsNumber(&leftInterpResult) ? bohRawExprInterpResultGetNumber(&leftInterpResult) : NULL;
    
    if (pLeftNumber && bohNumberIsZero(pLeftNumber)) {
        return bohRawExprInterpResultCreateNumberI64(false);
    }

    const bohRawExprInterpResult rightInterpResult = interpInterpretAstNode(pBinaryNode->pRightNode);
    BOH_ASSERT((bohRawExprInterpResultIsNumber(&rightInterpResult) || bohRawExprInterpResultIsString(&rightInterpResult)) && "Invalid right bohRawExprInterpResult type");

    const bohNumber* pRightNumber = bohRawExprInterpResultIsNumber(&rightInterpResult) ? bohRawExprInterpResultGetNumber(&rightInterpResult) : NULL;
    
    if (pRightNumber) {
        return bohRawExprInterpResultCreateNumberI64(!bohNumberIsZero(pRightNumber));
    }

    return bohRawExprInterpResultCreateNumberI64(true);
}


static bohRawExprInterpResult interpInterpretLogicalOr(const bohAstNodeBinary* pBinaryNode)
{
    BOH_ASSERT(pBinaryNode);

    const bohRawExprInterpResult leftInterpResult = interpInterpretAstNode(pBinaryNode->pLeftNode);
    BOH_ASSERT((bohRawExprInterpResultIsNumber(&leftInterpResult) || bohRawExprInterpResultIsString(&leftInterpResult)) && "Invalid left bohRawExprInterpResult type");

    const bohNumber* pLeftNumber = bohRawExprInterpResultIsNumber(&leftInterpResult) ? bohRawExprInterpResultGetNumber(&leftInterpResult) : NULL;
    
    if (pLeftNumber && !bohNumberIsZero(pLeftNumber)) {
        return bohRawExprInterpResultCreateNumberI64(true);
    }

    const bohRawExprInterpResult rightInterpResult = interpInterpretAstNode(pBinaryNode->pRightNode);
    BOH_ASSERT((bohRawExprInterpResultIsNumber(&rightInterpResult) || bohRawExprInterpResultIsString(&rightInterpResult)) && "Invalid right bohRawExprInterpResult type");

    const bohNumber* pRightNumber = bohRawExprInterpResultIsNumber(&rightInterpResult) ? bohRawExprInterpResultGetNumber(&rightInterpResult) : NULL;
    
    if (pRightNumber) {
        return bohRawExprInterpResultCreateNumberI64(!bohNumberIsZero(pRightNumber));
    }

    return bohRawExprInterpResultCreateNumberI64(true);
}


static bohRawExprInterpResult interpInterpretBinaryAstNode(const bohAstNode* pNode)
{
    BOH_ASSERT(pNode);

    const bohAstNodeBinary* pBinaryNode = bohAstNodeGetBinary(pNode);

    if (pBinaryNode->op == BOH_OP_AND) {
        return interpInterpretLogicalAnd(pBinaryNode);
    } else if (pBinaryNode->op == BOH_OP_OR) {
        return interpInterpretLogicalOr(pBinaryNode);
    }

    const bohRawExprInterpResult left = interpInterpretAstNode(pBinaryNode->pLeftNode);
    const bohRawExprInterpResult right = interpInterpretAstNode(pBinaryNode->pRightNode);

    BOH_ASSERT((bohRawExprInterpResultIsNumber(&left) || bohRawExprInterpResultIsString(&left)) && "Invalid left bohRawExprInterpResult type");
    BOH_ASSERT((bohRawExprInterpResultIsNumber(&right) || bohRawExprInterpResultIsString(&right)) && "Invalid right bohRawExprInterpResult type");

    const bohNumber* pLeftNumber = bohRawExprInterpResultIsNumber(&left) ? bohRawExprInterpResultGetNumber(&left) : NULL;
    const bohNumber* pRightNumber = bohRawExprInterpResultIsNumber(&right) ? bohRawExprInterpResultGetNumber(&right) : NULL;

    const bohBoharesString* pLeftStr = bohRawExprInterpResultIsString(&left) ? bohRawExprInterpResultGetString(&left) : NULL;
    const bohBoharesString* pRightStr = bohRawExprInterpResultIsString(&right) ? bohRawExprInterpResultGetString(&right) : NULL;

    const char* pOperatorStr = bohParsExprOperatorToStr(pBinaryNode->op);

    BOH_CHECK_INTERPRETER_COND(bohInterpAreInterpResultValuesSameType(&left, &right), pNode->line, pNode->column, 
        "invalid operation: %s %s %s", bohRawExprInterpResultTypeToStr(&left), pOperatorStr, bohRawExprInterpResultTypeToStr(&right));

    switch (pBinaryNode->op) {
        case BOH_OP_PLUS:
            if (pLeftNumber) {
                return bohRawExprInterpResultCreateNumber(bohNumberAdd(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                const bohBoharesString finalString = bohBoharesStringAdd(pLeftStr, pRightStr);
                return bohRawExprInterpResultCreateStringBoharesStringPtr(&finalString);
            }
            break;
        case BOH_OP_GREATER:
            if (pLeftNumber) {
                return bohRawExprInterpResultCreateNumberI64(bohNumberGreater(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprInterpResultCreateNumberI64(bohBoharesStringGreater(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_LESS:
            if (pLeftNumber) {
                return bohRawExprInterpResultCreateNumberI64(bohNumberLess(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprInterpResultCreateNumberI64(bohBoharesStringLess(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_NOT_EQUAL:
            if (pLeftNumber) {
                return bohRawExprInterpResultCreateNumberI64(bohNumberNotEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprInterpResultCreateNumberI64(bohBoharesStringNotEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_GEQUAL:
            if (pLeftNumber) {
                return bohRawExprInterpResultCreateNumberI64(bohNumberGreaterEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprInterpResultCreateNumberI64(bohBoharesStringGreaterEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_LEQUAL:
            if (pLeftNumber) {
                return bohRawExprInterpResultCreateNumberI64(bohNumberLessEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprInterpResultCreateNumberI64(bohBoharesStringLessEqual(pLeftStr, pRightStr));
            }
            break;
        case BOH_OP_EQUAL:
            if (pLeftNumber) {
                return bohRawExprInterpResultCreateNumberI64(bohNumberEqual(pLeftNumber, pRightNumber));
            } else if (pLeftStr) {
                return bohRawExprInterpResultCreateNumberI64(bohBoharesStringEqual(pLeftStr, pRightStr));
            }
            break;
        default:
            break;
    }

    BOH_CHECK_INTERPRETER_COND(bohRawExprInterpResultIsNumber(&left), pNode->line, pNode->column, "can't use binary %s operator with non numbers types", pOperatorStr);
    BOH_CHECK_INTERPRETER_COND(bohRawExprInterpResultIsNumber(&right), pNode->line, pNode->column, "can't use binary %s operator with non numbers types", pOperatorStr);

    if (bohParsIsBitwiseExprOperator(pBinaryNode->op)) {
        BOH_CHECK_INTERPRETER_COND(bohNumberIsI64(pLeftNumber) && bohNumberIsI64(pRightNumber), pNode->line, pNode->column, 
            "can't use %s bitwise operator with non integral types", pOperatorStr);
    }

    switch (pBinaryNode->op) {
        case BOH_OP_MINUS:
            return bohRawExprInterpResultCreateNumber(bohNumberSub(pLeftNumber, pRightNumber));
        case BOH_OP_MULT:
            return bohRawExprInterpResultCreateNumber(bohNumberMult(pLeftNumber, pRightNumber));
        case BOH_OP_DIV:
            BOH_CHECK_INTERPRETER_COND(!bohNumberIsZero(pRightNumber), pNode->line, pNode->column, "right operand of / is zero");
            return bohRawExprInterpResultCreateNumber(bohNumberDiv(pLeftNumber, pRightNumber));
        case BOH_OP_MOD:
            BOH_CHECK_INTERPRETER_COND(!bohNumberIsZero(pRightNumber), pNode->line, pNode->column, "right operand of % is zero");
            return bohRawExprInterpResultCreateNumber(bohNumberMod(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_AND:
            return bohRawExprInterpResultCreateNumber(bohNumberBitwiseAnd(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_OR:
            return bohRawExprInterpResultCreateNumber(bohNumberBitwiseOr(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_XOR:
            return bohRawExprInterpResultCreateNumber(bohNumberBitwiseXor(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_RSHIFT:
            return bohRawExprInterpResultCreateNumber(bohNumberBitwiseRShift(pLeftNumber, pRightNumber));
        case BOH_OP_BITWISE_LSHIFT:
            return bohRawExprInterpResultCreateNumber(bohNumberBitwiseLShift(pLeftNumber, pRightNumber));    
        default:
            BOH_ASSERT(false && "Invalid binary operator");
            return bohRawExprInterpResultCreateNumberI64(-1);
    }
}
#endif





static bohStmtIdx bohAstInterpretStmt(const bohAST* pAst, bohStmtIdx stmtIdx)
{
    BOH_ASSERT(pAst);

    const bohStmt* pStmt = bohAstGetStmtByIdx(pAst, stmtIdx);
    BOH_ASSERT(pStmt);

    switch(pStmt->type) {
        case BOH_STMT_TYPE_EMPTY:
            return stmtIdx;
        case BOH_STMT_TYPE_RAW_EXPR:
            break;
        case BOH_STMT_TYPE_PRINT:
            break;
        default:
            BOH_ASSERT(false && "Invalid statement type");
            return stmtIdx;
    }
}


static void bohAstInterpretStmts(const bohAST* pAst)
{
    BOH_ASSERT(pAst);
    
    const bohStmtStorage* pStmts = bohAstGetStmtsConst(pAst);
    const size_t stmtCount = bohDynArrayGetSize(pStmts);

    for (bohStmtIdx stmtIdx = 0; stmtIdx < stmtCount; ++stmtIdx) {
        stmtIdx = bohAstInterpretStmt(pAst, stmtIdx);
    }
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
