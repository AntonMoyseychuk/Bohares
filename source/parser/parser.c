#include "pch.h"

#include "core.h"

#include "parser.h"
#include "lexer/lexer.h"

#include "error.h"


#define BOH_PARSER_PRINT_ERROR(LINE, COLUMN, FMT, ...) \
    bohErrorsStatePrintError(stderr, bohErrorsStateGerCurrProcessingFileGlobal(), LINE, COLUMN, "PARSER ERROR", FMT, __VA_ARGS__)

#define BOH_PARSER_EXPECT(COND, LINE, COLUMN, FMT, ...)         \
    if (!(COND)) {                                              \
        BOH_PARSER_PRINT_ERROR(LINE, COLUMN, FMT, __VA_ARGS__); \
        bohErrorsStatePushParserErrorGlobal();                  \
    }


static void bohPtrDefContr(void* pElement)
{
    BOH_ASSERT(pElement);
    *((void**)pElement) = NULL;
}


static void bohPtrDestr(void* pElement)
{
    BOH_ASSERT(pElement);
    *((void**)pElement) = NULL;
}


static void bohPtrCopyFunc(void* pDst, const void* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);
    *((void**)pDst) = *((void**)pSrc);
}


static bohBoharesString parsGetUnescapedString(const bohBoharesString* pString)
{
    BOH_ASSERT(pString);

    const size_t strSize = bohBoharesStringGetSize(pString);

    bohBoharesString str = bohBoharesStringCreateString();
    if (strSize == 0) {
        return str;
    }

    bohBoharesStringResize(&str, strSize);

    size_t j = 0;

    for (size_t i = 0; i < strSize; ++i, ++j) {
        const char currSymb = bohBoharesStringAt(pString, i);

        if (currSymb == '\\') {
            const char nextSymb = bohBoharesStringAt(pString, i + 1);

            if (nextSymb == 't') {
                bohBoharesStringSetAt(&str, '\t', j);
                ++i;
            } else if (nextSymb == '\"') {
                bohBoharesStringSetAt(&str, '\"', j);
                ++i;
            } else if (nextSymb == '\'') {
                bohBoharesStringSetAt(&str, '\'', j);
                ++i;
            } else if (nextSymb == 'r') {
                bohBoharesStringSetAt(&str, '\r', j);
                ++i;
            } else if (nextSymb == 'n') {
                bohBoharesStringSetAt(&str, '\n', j);
                ++i;
            } else if (nextSymb == '\\') {
                bohBoharesStringSetAt(&str, '\\', j);
                ++i;
            } else {
                bohBoharesStringSetAt(&str, currSymb, j);
            }
        } else {
            bohBoharesStringSetAt(&str, currSymb, j);
        }
    }

    bohBoharesStringResize(&str, j);

    return str;
}


const char* bohParsExprOperatorToStr(bohExprOperator op)
{
    switch (op) {
        case BOH_OP_PLUS: return "+";
        case BOH_OP_PLUS_ASSIGN: return "+=";
        case BOH_OP_MINUS: return "-";
        case BOH_OP_MINUS_ASSIGN: return "-=";
        case BOH_OP_MULT: return "*";
        case BOH_OP_MULT_ASSIGN: return "*=";
        case BOH_OP_DIV: return "/";
        case BOH_OP_DIV_ASSIGN: return "/=";
        case BOH_OP_MOD: return "%";
        case BOH_OP_MOD_ASSIGN: return "%=";
        case BOH_OP_NOT: return "!";
        case BOH_OP_GREATER: return ">";
        case BOH_OP_LESS: return "<";
        case BOH_OP_NOT_EQUAL: return "!=";
        case BOH_OP_GEQUAL: return ">=";
        case BOH_OP_LEQUAL: return "<=";
        case BOH_OP_EQUAL: return "==";
        case BOH_OP_BITWISE_AND: return "&";
        case BOH_OP_BITWISE_AND_ASSIGN: return "&=";
        case BOH_OP_BITWISE_OR: return "|";
        case BOH_OP_BITWISE_OR_ASSIGN: return "|=";
        case BOH_OP_BITWISE_XOR: return "^";
        case BOH_OP_BITWISE_XOR_ASSIGN: return "^=";
        case BOH_OP_BITWISE_NOT: return "~";
        case BOH_OP_BITWISE_NOT_ASSIGN: return "~=";
        case BOH_OP_BITWISE_RSHIFT: return ">>";
        case BOH_OP_BITWISE_RSHIFT_ASSIGN: return ">>=";
        case BOH_OP_BITWISE_LSHIFT: return "<<";
        case BOH_OP_BITWISE_LSHIFT_ASSIGN: return "<<=";
        case BOH_OP_AND: return "and";
        case BOH_OP_OR: return "or";
        default:
            BOH_ASSERT_FAIL("Invalid operator type");
            return NULL;
    }
}


void bohValueExprDestroy(bohValueExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    switch (pExpr->type) {
        case BOH_VALUE_EXPR_TYPE_NUMBER:
            bohNumberSetI64(&pExpr->number, 0);
            break;
        case BOH_VALUE_EXPR_TYPE_STRING:
            bohBoharesStringDestroy(&pExpr->string);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid value expression type");
            break;
    }

    pExpr->type = BOH_VALUE_EXPR_TYPE_NUMBER;
}


void bohValueExprCreateInPlace(bohValueExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    pExpr->type = BOH_VALUE_EXPR_TYPE_NUMBER;
    pExpr->number = bohNumberCreateI64(0);
}


void bohValueExprCreateNumberInPlace(bohValueExpr* pExpr, bohNumber number)
{
    bohValueExprCreateNumberNumberPtrInPlace(pExpr, &number);
}


void bohValueExprCreateNumberNumberPtrInPlace(bohValueExpr* pExpr, const bohNumber* pNumber)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pNumber);

    pExpr->type = BOH_VALUE_EXPR_TYPE_NUMBER;
    bohNumberAssign(&pExpr->number, pNumber);
}


void bohValueExprCreateStringStringPtrInPlace(bohValueExpr* pExpr, const bohBoharesString* pString)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pString);

    pExpr->type = BOH_VALUE_EXPR_TYPE_STRING;
    bohBoharesStringAssign(&pExpr->string, pString);
}


void bohValueExprCreateStringStringMoveInPlace(bohValueExpr* pExpr, bohBoharesString* pString)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pString);

    pExpr->type = BOH_VALUE_EXPR_TYPE_STRING;
    bohBoharesStringMove(&pExpr->string, pString);
}


bool bohValueExprIsNumber(const bohValueExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type == BOH_VALUE_EXPR_TYPE_NUMBER;
}


bool bohValueExprIsNumberI64(const bohValueExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return bohValueExprIsNumber(pExpr) && bohNumberIsI64(&pExpr->number);
}


bool bohValueExprIsNumberF64(const bohValueExpr *pExpr)
{
    BOH_ASSERT(pExpr);
    return bohValueExprIsNumber(pExpr) && bohNumberIsF64(&pExpr->number);
}


bool bohValueExprIsString(const bohValueExpr *pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type == BOH_VALUE_EXPR_TYPE_STRING;
}


bohValueExprType bohValueExprGetType(const bohValueExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type;
}


const bohNumber *bohValueExprGetNumber(const bohValueExpr *pExpr)
{
    BOH_ASSERT(bohValueExprIsNumber(pExpr));
    return &pExpr->number;
}


const bohBoharesString* bohValueExprGetString(const bohValueExpr *pExpr)
{
    BOH_ASSERT(bohValueExprIsString(pExpr));
    return &pExpr->string;
}


bohValueExpr* bohValueExprAssign(bohValueExpr* pDst, const bohValueExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohValueExprDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_VALUE_EXPR_TYPE_NUMBER:
            bohNumberAssign(&pDst->number, &pSrc->number);
            break;
        case BOH_VALUE_EXPR_TYPE_STRING:
            bohBoharesStringAssign(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid value expr type");
            break;
    }

    return pDst;
}


bohValueExpr* bohValueExprMove(bohValueExpr* pDst, bohValueExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohValueExprDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_VALUE_EXPR_TYPE_NUMBER:
            bohNumberMove(&pDst->number, &pSrc->number);
            break;
        case BOH_VALUE_EXPR_TYPE_STRING:
            bohBoharesStringMove(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid value expr type");
            break;
    }

    pSrc->type = BOH_VALUE_EXPR_TYPE_NUMBER;

    return pDst;
}


void bohUnaryExprDestroy(bohUnaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    pExpr->pExpr = NULL;
    pExpr->op = BOH_OP_UNKNOWN;
}


void bohUnaryExprCreateOpExprInPlace(bohUnaryExpr* pExpr, bohExprOperator op, const bohExpr* pArgExpr)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pArgExpr);
    BOH_ASSERT(op != BOH_OP_UNKNOWN);

    pExpr->pExpr = pArgExpr;
    pExpr->op = op;
}


const bohExpr* bohUnaryExprGetExpr(const bohUnaryExpr *pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->pExpr;
}


bohExprOperator bohUnaryExprGetOperator(const bohUnaryExpr *pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->op;
}


bohUnaryExpr* bohUnaryExprAssign(bohUnaryExpr* pDst, const bohUnaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohUnaryExprDestroy(pDst);

    pDst->pExpr = pSrc->pExpr;
    pDst->op = pSrc->op;

    return pDst;
}


bohUnaryExpr* bohUnaryExprMove(bohUnaryExpr* pDst, bohUnaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohUnaryExprAssign(pDst, pSrc);
    
    pSrc->pExpr = NULL;
    pSrc->op = BOH_OP_UNKNOWN;

    return pDst;
}


void bohBinaryExprDestroy(bohBinaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    pExpr->pLeftExpr = NULL;
    pExpr->pRightExpr = NULL;
    pExpr->op = BOH_OP_UNKNOWN;
}


void bohBinaryExprCreateOpExprInPlace(bohBinaryExpr* pExpr, bohExprOperator op, const bohExpr* pLeftExpr, const bohExpr* pRightExpr)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pLeftExpr);
    BOH_ASSERT(pRightExpr);
    BOH_ASSERT(op != BOH_OP_UNKNOWN);

    pExpr->pLeftExpr = pLeftExpr;
    pExpr->pRightExpr = pRightExpr;
    pExpr->op = op;
}


const bohExpr* bohBinaryExprGetLeftExpr(const bohBinaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->pLeftExpr;
}


const bohExpr* bohBinaryExprGetRightExpr(const bohBinaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->pRightExpr;
}


bohExprOperator bohBinaryExprGetOperator(const bohBinaryExpr *pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->op;
}


bohBinaryExpr* bohBinaryExprAssign(bohBinaryExpr* pDst, const bohBinaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBinaryExprDestroy(pDst);

    pDst->pLeftExpr = pSrc->pLeftExpr;
    pDst->pRightExpr = pSrc->pRightExpr;
    pDst->op = pSrc->op;

    return pDst;
}


bohBinaryExpr* bohBinaryExprMove(bohBinaryExpr* pDst, bohBinaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBinaryExprAssign(pDst, pSrc);
    
    pSrc->pLeftExpr = NULL;
    pSrc->pRightExpr = NULL;
    pSrc->op = BOH_OP_UNKNOWN;

    return pDst;
}


void bohIdentifierExprDestroy(bohIdentifierExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    bohStringViewReset(&pExpr->name);
}


void bohIdentifierExprCreateInPlace(bohIdentifierExpr* pExpr, const bohStringView* pName)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pName);

    bohStringViewAssignStringViewPtr(&pExpr->name, pName);
}


const bohStringView* bohIdentifierExprGetName(const bohIdentifierExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return &pExpr->name;
}


bohIdentifierExpr* bohIdentifierExprAssign(bohIdentifierExpr* pDst, const bohIdentifierExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewAssignStringViewPtr(&pDst->name, &pSrc->name);

    return pDst;
}


bohIdentifierExpr* bohIdentifierExprMove(bohIdentifierExpr* pDst, bohIdentifierExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewMove(&pDst->name, &pSrc->name);

    return pDst;
}


static void bohExprSetLineColumnNmb(bohExpr* pExpr, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pExpr);

    pExpr->line = line;
    pExpr->column = column;
}


void bohExprDestroy(bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    switch (pExpr->type) {
        case BOH_EXPR_TYPE_VALUE:
            bohValueExprDestroy(&pExpr->valueExpr);
            break;
        case BOH_EXPR_TYPE_UNARY:
            bohUnaryExprDestroy(&pExpr->unaryExpr);
            break;
        case BOH_EXPR_TYPE_BINARY:
            bohBinaryExprDestroy(&pExpr->binaryExpr);
            break;
        case BOH_EXPR_TYPE_IDENTIFIER:
            bohIdentifierExprDestroy(&pExpr->identifierExpr);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid expression type");
            break;
    }

    pExpr->type = BOH_EXPR_TYPE_VALUE;
    bohExprSetLineColumnNmb(pExpr, 0, 0);
}


void bohExprCreateNumberValueExprInPlace(bohExpr* pExpr, bohNumber number, bohLineNmb line, bohColumnNmb column)
{
    bohExprCreateNumberValueExprPtrInPlace(pExpr, &number, line, column);
}


void bohExprCreateNumberValueExprPtrInPlace(bohExpr* pExpr, const bohNumber* pNumber, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pExpr);

    pExpr->type = BOH_EXPR_TYPE_VALUE;
    bohValueExprCreateNumberNumberPtrInPlace(&pExpr->valueExpr, pNumber);
    bohExprSetLineColumnNmb(pExpr, line, column);
}


void bohExprCreateStringValueExprInPlace(bohExpr* pExpr, const bohBoharesString* pString, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pExpr);

    pExpr->type = BOH_EXPR_TYPE_VALUE;
    bohValueExprCreateStringStringPtrInPlace(&pExpr->valueExpr, pString);
    bohExprSetLineColumnNmb(pExpr, line, column);
}


void bohExprCreateStringValueExprMoveInPlace(bohExpr* pExpr, bohBoharesString* pString, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pExpr);

    pExpr->type = BOH_EXPR_TYPE_VALUE;
    bohValueExprCreateStringStringMoveInPlace(&pExpr->valueExpr, pString);
    bohExprSetLineColumnNmb(pExpr, line, column);
}


void bohExprCreateUnaryExprInPlace(bohExpr* pExpr, bohExprOperator op, bohExpr* pArgExpr, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pExpr);

    pExpr->type = BOH_EXPR_TYPE_UNARY;
    bohUnaryExprCreateOpExprInPlace(&pExpr->unaryExpr, op, pArgExpr);
    bohExprSetLineColumnNmb(pExpr, line, column);
}


void bohExprCreateBinaryExprInPlace(bohExpr* pExpr, bohExprOperator op, bohExpr* pLeftArgExpr, bohExpr* pRightArgExpr, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pLeftArgExpr);
    BOH_ASSERT(pRightArgExpr);

    pExpr->type = BOH_EXPR_TYPE_BINARY;
    bohBinaryExprCreateOpExprInPlace(&pExpr->binaryExpr, op, pLeftArgExpr, pRightArgExpr);
    bohExprSetLineColumnNmb(pExpr, line, column);
}


void bohExprCreateIdentifierExprInPlace(bohExpr* pExpr, const bohStringView* pName, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pName);

    pExpr->type = BOH_EXPR_TYPE_IDENTIFIER;
    bohIdentifierExprCreateInPlace(&pExpr->identifierExpr, pName);
    bohExprSetLineColumnNmb(pExpr, line, column);
}


bool bohExprIsValueExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type == BOH_EXPR_TYPE_VALUE;
}


bool bohExprIsUnaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type == BOH_EXPR_TYPE_UNARY;
}


bool bohExprIsBinaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type == BOH_EXPR_TYPE_BINARY;
}


bool bohExprIsIdentifierExpr(const bohExpr *pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type == BOH_EXPR_TYPE_IDENTIFIER;
}


const bohValueExpr* bohExprGetValueExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsValueExpr(pExpr));
    return &pExpr->valueExpr;
}


const bohUnaryExpr* bohExprGetUnaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsUnaryExpr(pExpr));
    return &pExpr->unaryExpr;
}


const bohBinaryExpr* bohExprGetBinaryExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsBinaryExpr(pExpr));
    return &pExpr->binaryExpr;
}

const bohIdentifierExpr* bohExprGetIdentifierExpr(const bohExpr* pExpr)
{
    BOH_ASSERT(bohExprIsIdentifierExpr(pExpr));
    return &pExpr->identifierExpr;
}


bohExprType bohExprGetType(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->type;
}


bohLineNmb bohExprGetLine(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->line;
}


bohColumnNmb bohExprGetColumn(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->column;
}


bohExpr* bohExprAssign(bohExpr* pDst, const bohExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohExprDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_EXPR_TYPE_VALUE:
            bohValueExprAssign(&pDst->valueExpr, &pSrc->valueExpr);
            break;
        case BOH_EXPR_TYPE_UNARY:
            bohUnaryExprAssign(&pDst->unaryExpr, &pSrc->unaryExpr);
            break;
        case BOH_EXPR_TYPE_BINARY:
            bohBinaryExprAssign(&pDst->binaryExpr, &pSrc->binaryExpr);
            break;
        case BOH_EXPR_TYPE_IDENTIFIER:
            bohIdentifierExprAssign(&pDst->identifierExpr, &pSrc->identifierExpr);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid expression type");
            break;
    }

    bohExprSetLineColumnNmb(pDst, pSrc->line, pSrc->column);

    return pDst;
}


bohExpr* bohExprMove(bohExpr* pDst, bohExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohExprDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_EXPR_TYPE_VALUE:
            bohValueExprMove(&pDst->valueExpr, &pSrc->valueExpr);
            break;
        case BOH_EXPR_TYPE_UNARY:
            bohUnaryExprMove(&pDst->unaryExpr, &pSrc->unaryExpr);
            break;
        case BOH_EXPR_TYPE_BINARY:
            bohBinaryExprMove(&pDst->binaryExpr, &pSrc->binaryExpr);
            break;
        case BOH_EXPR_TYPE_IDENTIFIER:
            bohIdentifierExprMove(&pDst->identifierExpr, &pSrc->identifierExpr);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid expression type");
            break;
    }

    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    pSrc->type = BOH_EXPR_TYPE_VALUE;
    bohExprSetLineColumnNmb(pSrc, 0, 0);

    return pDst;
}


void bohPrintStmtDestroy(bohPrintStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    pStmt->pArgExpr = NULL;
}


void bohPrintStmtCreateInPlace(bohPrintStmt* pStmt, const bohExpr* pArgExpr)
{
    BOH_ASSERT(pStmt);
    BOH_ASSERT(pArgExpr);

    pStmt->pArgExpr = pArgExpr;
}


const bohExpr* bohPrintStmtGetArgExpr(const bohPrintStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->pArgExpr;
}


bohPrintStmt* bohPrintStmtAssign(bohPrintStmt* pDst, const bohPrintStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohPrintStmtDestroy(pDst);
    pDst->pArgExpr = pSrc->pArgExpr;

    return pDst;
}


bohPrintStmt* bohPrintStmtMove(bohPrintStmt* pDst, bohPrintStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohPrintStmtAssign(pDst, pSrc);
    pSrc->pArgExpr = NULL;

    return pDst;
}


void bohIfStmtDestroy(bohIfStmt* pStmt)
{
    BOH_ASSERT(pStmt);

    pStmt->pCondExpr = NULL;
    bohDynArrayDestroy(&pStmt->thenStmtPtrs);
    bohDynArrayDestroy(&pStmt->elseStmtPtrs);
}


void bohIfStmtCreateInPlace(bohIfStmt* pStmt, const bohExpr* pCondExpr, bohDynArray* pThenStmtPtrs, bohDynArray* pElseStmtPtrs)
{
    BOH_ASSERT(pStmt);
    BOH_ASSERT(pCondExpr);

    pStmt->pCondExpr = pCondExpr;
    bohDynArrayMove(&pStmt->thenStmtPtrs, pThenStmtPtrs);
    bohDynArrayMove(&pStmt->elseStmtPtrs, pElseStmtPtrs);
}


const bohExpr* bohIfStmtGetCondExpr(const bohIfStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->pCondExpr;
}


const bohDynArray* bohIfStmtGetThenStmts(const bohIfStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return &pStmt->thenStmtPtrs;
}


const bohDynArray* bohIfStmtGetElseStmts(const bohIfStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return &pStmt->elseStmtPtrs;
}


size_t bohIfStmtGetThenStmtsCount(const bohIfStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return bohDynArrayGetSize(&pStmt->thenStmtPtrs);
}


size_t bohIfStmtGetElseStmtsCount(const bohIfStmt *pStmt)
{
    BOH_ASSERT(pStmt);
    return bohDynArrayGetSize(&pStmt->elseStmtPtrs);
}


const bohStmt* bohIfStmtGetThenStmtAt(const bohIfStmt* pStmt, size_t index)
{
    BOH_ASSERT(pStmt);
    return *BOH_DYN_ARRAY_AT_CONST(bohStmt*, &pStmt->thenStmtPtrs, index);
}


const bohStmt* bohIfStmtGetElseStmtAt(const bohIfStmt* pStmt, size_t index)
{
    BOH_ASSERT(pStmt);
    return *BOH_DYN_ARRAY_AT_CONST(bohStmt*, &pStmt->elseStmtPtrs, index);
}


bohIfStmt* bohIfStmtAssign(bohIfStmt* pDst, const bohIfStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->pCondExpr = pSrc->pCondExpr;
    bohDynArrayAssign(&pDst->thenStmtPtrs, &pSrc->thenStmtPtrs);
    bohDynArrayAssign(&pDst->elseStmtPtrs, &pSrc->elseStmtPtrs);

    return pDst;
}


bohIfStmt* bohIfStmtMove(bohIfStmt* pDst, bohIfStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->pCondExpr = pSrc->pCondExpr;
    bohDynArrayMove(&pDst->thenStmtPtrs, &pSrc->thenStmtPtrs);
    bohDynArrayMove(&pDst->elseStmtPtrs, &pSrc->elseStmtPtrs);

    pSrc->pCondExpr = NULL;

    return pDst;
}


void bohVarDeclStmtDestroy(bohVarDeclStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    bohStringViewReset(&pStmt->name);
}


void bohVarDeclStmtCreateInPlace(bohVarDeclStmt* pStmt, const bohStringView* pName)
{
    BOH_ASSERT(pStmt);
    BOH_ASSERT(pName);

    bohStringViewAssignStringViewPtr(&pStmt->name, pName);
}


const bohStringView* bohVarDeclStmtGetName(const bohVarDeclStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return &pStmt->name;
}


bohVarDeclStmt* bohVarDeclStmtAssign(bohVarDeclStmt* pDst, const bohVarDeclStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewAssignStringViewPtr(&pDst->name, &pSrc->name);

    return pDst;
}


bohVarDeclStmt* bohVarDeclStmtMove(bohVarDeclStmt* pDst, bohVarDeclStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewMove(&pDst->name, &pSrc->name);

    return pDst;
}


void bohAssignmentStmtDestroy(bohAssignmentStmt* pStmt)
{
    BOH_ASSERT(pStmt);

    pStmt->pLeft = NULL;
    pStmt->pRight = NULL;
}


void bohAssignmentStmtCreateInPlace(bohAssignmentStmt* pStmt, const bohExpr* pLeft, const bohExpr* pRight)
{
    BOH_ASSERT(pStmt);

    pStmt->pLeft = pLeft;
    pStmt->pRight = pRight;
}


const bohExpr* bohAssignmentStmtGetLeftExpr(const bohAssignmentStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->pLeft;
}


const bohExpr* bohAssignmentStmtGetRightExpr(const bohAssignmentStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->pRight;
}


bohAssignmentStmt* bohAssignmentStmtAssign(bohAssignmentStmt* pDst, const bohAssignmentStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->pLeft = pSrc->pLeft;
    pDst->pRight = pSrc->pRight;

    return pDst;
}


bohAssignmentStmt* bohAssignmentStmtMove(bohAssignmentStmt* pDst, bohAssignmentStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->pLeft = pSrc->pLeft;
    pDst->pRight = pSrc->pRight;

    pSrc->pLeft = NULL;
    pSrc->pRight = NULL;
    
    return pDst;
}


static void bohStmtSetLineColumnNmb(bohStmt* pStmt, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pStmt);

    pStmt->line = line;
    pStmt->column = column;
}


void bohStmtDestroy(bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);

    switch (pStmt->type) {
        case BOH_STMT_TYPE_EMPTY:
            break;
        case BOH_STMT_TYPE_PRINT:
            bohPrintStmtDestroy(&pStmt->printStmt);
            break;
        case BOH_STMT_TYPE_IF:
            bohIfStmtDestroy(&pStmt->ifStmt);
            break;
        case BOH_STMT_TYPE_VAR_DECL:
            bohVarDeclStmtDestroy(&pStmt->varDeclStmt);
            break;
        case BOH_STMT_TYPE_ASSIGNMENT:
            bohAssignmentStmtDestroy(&pStmt->assignStmt);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid statement type");
            break;
    }

    pStmt->type = BOH_STMT_TYPE_EMPTY;
    bohStmtSetLineColumnNmb(pStmt, 0, 0);
}


void bohStmtCreatePrintInPlace(bohStmt* pStmt, const bohExpr* pArgExpr, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pStmt);

    pStmt->type = BOH_STMT_TYPE_PRINT;
    bohPrintStmtCreateInPlace(&pStmt->printStmt, pArgExpr);
    bohStmtSetLineColumnNmb(pStmt, line, column);
}


void bohStmtCreateIfInPlace(bohStmt* pStmt, const bohExpr* pCondExpr, bohDynArray* pThenStmtPtrs, bohDynArray* pElseStmtPtrs, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pStmt);

    pStmt->type = BOH_STMT_TYPE_IF;
    bohIfStmtCreateInPlace(&pStmt->ifStmt, pCondExpr, pThenStmtPtrs, pElseStmtPtrs);
    bohStmtSetLineColumnNmb(pStmt, line, column);
}


void bohStmtCreateAssignInPlace(bohStmt* pStmt, const bohExpr* pLeft, const bohExpr* pRight, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pStmt);

    pStmt->type = BOH_STMT_TYPE_ASSIGNMENT;
    bohAssignmentStmtCreateInPlace(&pStmt->assignStmt, pLeft, pRight);
    bohStmtSetLineColumnNmb(pStmt, line, column);
}


void bohStmtCreateVarDeclInPlace(bohStmt* pStmt, const bohStringView* pName, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pStmt);
    BOH_ASSERT(pName);

    pStmt->type = BOH_STMT_TYPE_VAR_DECL;
    bohVarDeclStmtCreateInPlace(&pStmt->varDeclStmt, pName);
    bohStmtSetLineColumnNmb(pStmt, line, column);
}


bool bohStmtIsEmpty(const bohStmt *pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type == BOH_STMT_TYPE_EMPTY;
}


bool bohStmtIsPrint(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type == BOH_STMT_TYPE_PRINT;
}


bool bohStmtIsIf(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type == BOH_STMT_TYPE_IF;
}


bool bohStmtIsAssignment(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type == BOH_STMT_TYPE_ASSIGNMENT;
}


bool bohStmtIsVarDecl(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type == BOH_STMT_TYPE_VAR_DECL;
}


const bohPrintStmt* bohStmtGetPrint(const bohStmt* pStmt)
{
    BOH_ASSERT(bohStmtIsPrint(pStmt));
    return &pStmt->printStmt;
}


const bohIfStmt* bohStmtGetIf(const bohStmt* pStmt)
{
    BOH_ASSERT(bohStmtIsIf(pStmt));
    return &pStmt->ifStmt;
}


const bohAssignmentStmt* bohStmtGetAssignment(const bohStmt* pStmt)
{
    BOH_ASSERT(bohStmtIsAssignment(pStmt));
    return &pStmt->assignStmt;
}


const bohVarDeclStmt* bohStmtGetVarDecl(const bohStmt* pStmt)
{
    BOH_ASSERT(bohStmtIsVarDecl(pStmt));
    return &pStmt->varDeclStmt;
}


bohStmt* bohStmtAssign(bohStmt* pDst, const bohStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStmtDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_STMT_TYPE_EMPTY:
            break;
        case BOH_STMT_TYPE_PRINT:
            bohPrintStmtAssign(&pDst->printStmt, &pSrc->printStmt);
            break;
        case BOH_STMT_TYPE_IF:
            bohIfStmtAssign(&pDst->ifStmt, &pSrc->ifStmt);
            break;
        case BOH_STMT_TYPE_VAR_DECL:
            bohVarDeclStmtAssign(&pDst->varDeclStmt, &pSrc->varDeclStmt);
            break;
        case BOH_STMT_TYPE_ASSIGNMENT:
            bohAssignmentStmtAssign(&pDst->assignStmt, &pSrc->assignStmt);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid statement type");
            break;
    }

    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


bohStmt* bohStmtMove(bohStmt* pDst, bohStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStmtDestroy(pDst);

    pDst->type = pSrc->type;

    switch (pSrc->type) {
        case BOH_STMT_TYPE_EMPTY:
            break;
        case BOH_STMT_TYPE_PRINT:
            bohPrintStmtMove(&pDst->printStmt, &pSrc->printStmt);
            break;
        case BOH_STMT_TYPE_IF:
            bohIfStmtMove(&pDst->ifStmt, &pSrc->ifStmt);
            break;
        case BOH_STMT_TYPE_VAR_DECL:
            bohVarDeclStmtMove(&pDst->varDeclStmt, &pSrc->varDeclStmt);
            break;
        case BOH_STMT_TYPE_ASSIGNMENT:
            bohAssignmentStmtMove(&pDst->assignStmt, &pSrc->assignStmt);
            break;
        default:
            BOH_ASSERT_FAIL("Invalid statement type");
            break;
    }

    bohStmtSetLineColumnNmb(pDst, pSrc->line, pSrc->column);

    pSrc->type = BOH_STMT_TYPE_EMPTY;
    bohStmtSetLineColumnNmb(pSrc, 0, 0);

    return pDst;
}


bohStmtType bohStmtGetType(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type;
}


bohLineNmb bohStmtGetLine(const bohStmt *pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->line;
}


bohColumnNmb bohStmtGetColumn(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->column;
}


bool bohParsIsBitwiseExprOperator(bohExprOperator op)
{
    return 
        op == BOH_OP_BITWISE_AND || 
        op == BOH_OP_BITWISE_AND_ASSIGN || 
        op == BOH_OP_BITWISE_OR || 
        op == BOH_OP_BITWISE_OR_ASSIGN || 
        op == BOH_OP_BITWISE_XOR || 
        op == BOH_OP_BITWISE_XOR_ASSIGN ||
        op == BOH_OP_BITWISE_NOT ||
        op == BOH_OP_BITWISE_NOT_ASSIGN ||
        op == BOH_OP_BITWISE_RSHIFT ||
        op == BOH_OP_BITWISE_RSHIFT_ASSIGN ||
        op == BOH_OP_BITWISE_LSHIFT ||
        op == BOH_OP_BITWISE_LSHIFT_ASSIGN;
}


static bohExprOperator parsTokenTypeToExprOperator(bohTokenType tokenType)
{
    switch (tokenType) {
        case BOH_TOKEN_TYPE_PLUS:                   return BOH_OP_PLUS;
        case BOH_TOKEN_TYPE_PLUS_ASSIGN:            return BOH_OP_PLUS_ASSIGN;
        case BOH_TOKEN_TYPE_MINUS:                  return BOH_OP_MINUS;
        case BOH_TOKEN_TYPE_MINUS_ASSIGN:           return BOH_OP_MINUS_ASSIGN;
        case BOH_TOKEN_TYPE_MULT:                   return BOH_OP_MULT;
        case BOH_TOKEN_TYPE_MULT_ASSIGN:            return BOH_OP_MULT_ASSIGN;
        case BOH_TOKEN_TYPE_DIV:                    return BOH_OP_DIV;
        case BOH_TOKEN_TYPE_DIV_ASSIGN:             return BOH_OP_DIV_ASSIGN;
        case BOH_TOKEN_TYPE_MOD:                    return BOH_OP_MOD;
        case BOH_TOKEN_TYPE_MOD_ASSIGN:             return BOH_OP_MOD_ASSIGN;
        case BOH_TOKEN_TYPE_NOT:                    return BOH_OP_NOT;
        case BOH_TOKEN_TYPE_GREATER:                return BOH_OP_GREATER;
        case BOH_TOKEN_TYPE_LESS:                   return BOH_OP_LESS;
        case BOH_TOKEN_TYPE_NOT_EQUAL:              return BOH_OP_NOT_EQUAL;
        case BOH_TOKEN_TYPE_GEQUAL:                 return BOH_OP_GEQUAL;
        case BOH_TOKEN_TYPE_LEQUAL:                 return BOH_OP_LEQUAL;
        case BOH_TOKEN_TYPE_EQUAL:                  return BOH_OP_EQUAL;
        case BOH_TOKEN_TYPE_BITWISE_AND:            return BOH_OP_BITWISE_AND;
        case BOH_TOKEN_TYPE_BITWISE_AND_ASSIGN:     return BOH_OP_BITWISE_AND_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_OR:             return BOH_OP_BITWISE_OR;
        case BOH_TOKEN_TYPE_BITWISE_OR_ASSIGN:      return BOH_OP_BITWISE_OR_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_XOR:            return BOH_OP_BITWISE_XOR;
        case BOH_TOKEN_TYPE_BITWISE_XOR_ASSIGN:     return BOH_OP_BITWISE_XOR_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_NOT:            return BOH_OP_BITWISE_NOT;
        case BOH_TOKEN_TYPE_BITWISE_NOT_ASSIGN:     return BOH_OP_BITWISE_NOT_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_RSHIFT:         return BOH_OP_BITWISE_RSHIFT;
        case BOH_TOKEN_TYPE_BITWISE_RSHIFT_ASSIGN:  return BOH_OP_BITWISE_RSHIFT_ASSIGN;
        case BOH_TOKEN_TYPE_BITWISE_LSHIFT:         return BOH_OP_BITWISE_LSHIFT;
        case BOH_TOKEN_TYPE_BITWISE_LSHIFT_ASSIGN:  return BOH_OP_BITWISE_LSHIFT_ASSIGN;
        case BOH_TOKEN_TYPE_AND:                    return BOH_OP_AND;
        case BOH_TOKEN_TYPE_OR:                     return BOH_OP_OR;
        default:                                    return BOH_OP_UNKNOWN;
    }
}


static const bohToken* parsPeekCurrToken(const bohParser* pParser)
{
    BOH_ASSERT(pParser);
    return BOH_DYN_ARRAY_AT_CONST(bohToken, pParser->pTokenStorage, pParser->currTokenIdx);
}


static const bohToken* parsPeekPrevToken(const bohParser* pParser)
{
    BOH_ASSERT(pParser);
    BOH_ASSERT(pParser->currTokenIdx > 0);

    const size_t prevTokenIdx = pParser->currTokenIdx - 1;
    BOH_ASSERT(prevTokenIdx < bohDynArrayGetSize(pParser->pTokenStorage));

    return BOH_DYN_ARRAY_AT_CONST(bohToken, pParser->pTokenStorage, prevTokenIdx);
}


static const bohToken* parsPeekNextToken(const bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const size_t nextTokenIdx = pParser->currTokenIdx + 1;

    if (nextTokenIdx < bohDynArrayGetSize(pParser->pTokenStorage)) {
        return BOH_DYN_ARRAY_AT_CONST(bohToken, pParser->pTokenStorage, nextTokenIdx);
    }

    return NULL;
}


// Returns current token and advance token index
static const bohToken* parsAdvanceToken(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    ++pParser->currTokenIdx;
    return parsPeekPrevToken(pParser);
}


static bool parsIsCurrTokenMatch(bohParser* pParser, bohTokenType type)
{
    BOH_ASSERT(pParser);

    if (pParser->currTokenIdx >= bohDynArrayGetSize(pParser->pTokenStorage)) {
        return false;
    }

    if (parsPeekCurrToken(pParser)->type != type) {
        return false;
    }

    parsAdvanceToken(pParser);
    return true;
}


static bohExpr* parsParsExpr(bohParser* pParser);


// <primary> = <integer> | <float> | <string> | <identifier> | '(' <expr> ')' 
static bohExpr* parsParsPrimary(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_TRUE)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        const bohToken* pPrevToken = parsPeekPrevToken(pParser);
        bohExprCreateNumberValueExprInPlace(pPrimaryExpr, bohNumberCreateI64(1), pPrevToken->line, pPrevToken->column);
        
        return pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FALSE)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        const bohToken* pPrevToken = parsPeekPrevToken(pParser);
        bohExprCreateNumberValueExprInPlace(pPrimaryExpr, bohNumberCreateI64(0), pPrevToken->line, pPrevToken->column);

        return pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_INTEGER)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        const bohToken* pPrevToken = parsPeekPrevToken(pParser);

        const int64_t value = _atoi64(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        bohExprCreateNumberValueExprInPlace(pPrimaryExpr, bohNumberCreateI64(value), pPrevToken->line, pPrevToken->column);

        return pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FLOAT)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        const bohToken* pPrevToken = parsPeekPrevToken(pParser);

        const double value = atof(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        bohExprCreateNumberValueExprInPlace(pPrimaryExpr, bohNumberCreateF64(value), pPrevToken->line, pPrevToken->column);
        
        return pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_STRING)) {
        bohBoharesString lexeme = bohBoharesStringCreateStringViewStringViewPtr(&parsPeekPrevToken(pParser)->lexeme);
        bohBoharesString unescapedLexeme = parsGetUnescapedString(&lexeme);

        bohBoharesStringDestroy(&lexeme);
        
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);
        const bohToken* pPrevToken = parsPeekPrevToken(pParser);

        bohExprCreateStringValueExprMoveInPlace(pPrimaryExpr, &unescapedLexeme, pPrevToken->line, pPrevToken->column);

        return pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_IDENTIFIER)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);        
        
        const bohToken* pPrevToken = parsPeekPrevToken(pParser);
        bohExprCreateIdentifierExprInPlace(pPrimaryExpr, &pPrevToken->lexeme, pPrevToken->line, pPrevToken->column);

        return pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LPAREN)) {
        const uint32_t line = parsPeekCurrToken(pParser)->line;
        const uint32_t column = parsPeekCurrToken(pParser)->column;

        bohExpr* pExpr = parsParsExpr(pParser);
        BOH_PARSER_EXPECT(parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RPAREN), line, column, "missed closing \')\'");
        
        return pExpr;
    }
    
    return NULL;
}


// <unary> = ('+' | '-' | '~' | '!') <unary> | <primary>
static bohExpr* parsParsUnary(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PLUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_NOT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_NOT)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);

        bohExpr* pOperandExpr = parsParsUnary(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, "unknown unary operator: %.*s", 
            bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));
        
        bohExpr* pUnaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateUnaryExprInPlace(pUnaryExpr, op, pOperandExpr, pOperatorToken->line, pOperatorToken->column);

        return pUnaryExpr;
    }

    return parsParsPrimary(pParser);
}


// <multiplication> = <unary> (('*' | '/' | '%') <unary>)*
static bohExpr* parsParsMultiplication(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsUnary(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MULT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_DIV) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MOD)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsUnary(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown term operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <addition> = <multiplication> (('+' | '-') <multiplication>)*
static bohExpr* parsParsAddition(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsMultiplication(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PLUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsMultiplication(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <bitwise_shift> = <addition> (('<<' | '>>') <addition>)*
static bohExpr* parsParsBitwiseShift(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsAddition(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_LSHIFT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_RSHIFT)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsAddition(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <comparison> = <bitwise_shift> (('>' | '>=' | '<' | '<=') <bitwise_shift>)*
static bohExpr* parsParsComparison(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsBitwiseShift(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_GREATER) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LESS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_GEQUAL) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LEQUAL)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsBitwiseShift(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <equality> = <comparison> (('!=' | '==') <comparison>)*
static bohExpr* parsParsEquality(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsComparison(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_NOT_EQUAL) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_EQUAL)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsComparison(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <bitwise_and> = <equality> (('&') <equality>)*
static bohExpr* parsParsBitwiseAnd(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsEquality(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_AND)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsEquality(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <bitwise_xor> = <bitwise_and> (('^') <bitwise_and>)*
static bohExpr* parsParsBitwiseXor(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsBitwiseAnd(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_XOR)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsBitwiseAnd(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <bitwise_or> = <bitwise_xor> (('|') <bitwise_xor>)*
static bohExpr* parsParsBitwiseOr(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsBitwiseXor(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_OR)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsBitwiseXor(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <and> = <bitwise_or> (('and' | '&&') <bitwise_or>)*
static bohExpr* parsParsAnd(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsBitwiseOr(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_AND)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsBitwiseOr(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


// <or> = <and> (('or' | '||') <and>)*
static bohExpr* parsParsOr(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr* pLeftExpr = parsParsAnd(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_OR)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        bohExpr* pRightExpr = parsParsAnd(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        bohExprCreateBinaryExprInPlace(pBinaryExpr, op, pLeftExpr, pRightExpr, pOperatorToken->line, pOperatorToken->column);

        pLeftExpr = pBinaryExpr;
    }

    return pLeftExpr;
}


static bohExpr* parsParsExpr(bohParser* pParser)
{
    return parsParsOr(pParser);
}


static bohStmt* parsParsNextStmt(bohParser* pParser);


// <print_stmt> = "print" <stmt>
static bohStmt* parsParsPrintStmt(bohParser* pParser)
{
    BOH_ASSERT(pParser);
    
    const bohToken* pCurrToken = parsPeekCurrToken(pParser);
    const bohExpr* pArgExpr = parsParsExpr(pParser);

    bohStmt* pPrintStmt = bohAstAllocateStmt(&pParser->ast);
    bohStmtCreatePrintInPlace(pPrintStmt, pArgExpr, pCurrToken->line, pCurrToken->column);

    return pPrintStmt;
}


// <if_stmt> = "if" <stmt> { (<stmt>)* } ("else" {(<stmt>)*})?
static bohStmt* parsParsIfStmt(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const bohToken* pCurrToken = parsPeekCurrToken(pParser);
    const bohExpr* pCondExpr = parsParsExpr(pParser);
    
    BOH_PARSER_EXPECT(parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LCURLY), parsPeekPrevToken(pParser)->line, parsPeekPrevToken(pParser)->column, 
        "expected opening \'{\' in \'if\' statement block");

    bohDynArray thenStmtsPtrs = BOH_DYN_ARRAY_CREATE(bohStmt*, bohPtrDefContr, bohPtrDestr, bohPtrCopyFunc);
    bohDynArray elseStmtsPtrs = BOH_DYN_ARRAY_CREATE(bohStmt*, bohPtrDefContr, bohPtrDestr, bohPtrCopyFunc);

    const size_t tokensCount = bohDynArrayGetSize(pParser->pTokenStorage);

    static const size_t BOH_PLEALLOCATED_INNER_STMT_COUNT = 16;

    if (!parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RCURLY)) {
        bohDynArrayReserve(&thenStmtsPtrs, BOH_PLEALLOCATED_INNER_STMT_COUNT);

        while(pParser->currTokenIdx < tokensCount && !parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RCURLY)) {
            bohStmt** ppThenStmt = (bohStmt**)bohDynArrayPushBackDummy(&thenStmtsPtrs);
            *ppThenStmt = parsParsNextStmt(pParser);
        }

        const bohToken* pRCurlyToken = parsPeekPrevToken(pParser);
        BOH_PARSER_EXPECT(pRCurlyToken->type == BOH_TOKEN_TYPE_RCURLY, pRCurlyToken->line, pRCurlyToken->column, 
            "expected closing \'}\' in \'if\' statement block");
    }

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_ELSE)) {
        bohDynArrayReserve(&elseStmtsPtrs, BOH_PLEALLOCATED_INNER_STMT_COUNT);

        BOH_PARSER_EXPECT(parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LCURLY), parsPeekPrevToken(pParser)->line, parsPeekPrevToken(pParser)->column, 
            "expected opening \'{\' in \'else\' statement block");
        
        while(pParser->currTokenIdx < tokensCount && !parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RCURLY)) {
            bohStmt** ppElseStmt = (bohStmt**)bohDynArrayPushBackDummy(&elseStmtsPtrs);
            *ppElseStmt = parsParsNextStmt(pParser);
        }

        const bohToken* pRCurlyToken = parsPeekPrevToken(pParser);
        BOH_PARSER_EXPECT(pRCurlyToken->type == BOH_TOKEN_TYPE_RCURLY, pRCurlyToken->line, pRCurlyToken->column, 
            "expected closing \'}\' in \'else\' statement block");
    }

    bohStmt* pIfStmt = bohAstAllocateStmt(&pParser->ast);
    bohStmtCreateIfInPlace(pIfStmt, pCondExpr, &thenStmtsPtrs, &elseStmtsPtrs, pCurrToken->line, pCurrToken->column);

    return pIfStmt;
}


// <var_decl_stmt> = "var" name
static bohStmt* parsParsVarDeclStmt(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const bohToken* pCurrToken = parsPeekCurrToken(pParser);
    BOH_PARSER_EXPECT(pCurrToken->type == BOH_TOKEN_TYPE_IDENTIFIER, pCurrToken->line, pCurrToken->line, "expected an identifier");

    const bohToken* pNextToken = parsPeekNextToken(pParser);
    if (pNextToken->type != BOH_TOKEN_TYPE_ASSIGNMENT) {
        // If next token is '=' than we don't have to advance curr token position.
        parsAdvanceToken(pParser);
    }

    bohStmt* pVarDeclStmt = bohAstAllocateStmt(&pParser->ast);
    bohStmtCreateVarDeclInPlace(pVarDeclStmt, &pCurrToken->lexeme, pCurrToken->line, pCurrToken->column);
    return pVarDeclStmt;
}


static bohStmt* parsParsNextStmt(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PRINT)) {
        return parsParsPrintStmt(pParser);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_IF)) {
        return parsParsIfStmt(pParser);
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_VAR)) {
        return parsParsVarDeclStmt(pParser);
    } else {
        const bohToken* pCurrToken = parsPeekCurrToken(pParser);

        const bohExpr* pLeftExpr = parsParsExpr(pParser);

        if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_ASSIGNMENT)) {
            const bohExpr* pRightExpr = parsParsExpr(pParser);

            bohStmt* pAssignmentStmt = bohAstAllocateStmt(&pParser->ast);
            bohStmtCreateAssignInPlace(pAssignmentStmt, pLeftExpr, pRightExpr, pCurrToken->line, pCurrToken->column);
            return pAssignmentStmt;
        }

        BOH_ASSERT_FAIL("Invalid token type");
        return NULL;
    }
}


static void parsParsStmts(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const bohTokenStorage* pTokenStorage = pParser->pTokenStorage;
    const size_t tokensCount = bohDynArrayGetSize(pTokenStorage);
    
    while(pParser->currTokenIdx < tokensCount) {
        bohAstPushStmtPtr(&pParser->ast, parsParsNextStmt(pParser));
    }
}


void bohAstDestroy(bohAST* pAST)
{
    BOH_ASSERT(pAST);

    bohDynArrayDestroy(&pAST->stmtPtrsStorage);

    bohArenaAllocatorDestroy(&pAST->stmtMemArena);
    bohArenaAllocatorDestroy(&pAST->epxrMemArena);
}


bohAST bohAstCreate(void)
{
    bohAST ast = {0};

    ast.stmtPtrsStorage = BOH_DYN_ARRAY_CREATE(bohStmt*, bohPtrDefContr, bohPtrDestr, bohPtrCopyFunc);

    ast.stmtMemArena = bohArenaAllocatorCreate((size_t)1 << 20);
    ast.epxrMemArena = bohArenaAllocatorCreate((size_t)1 << 20);

    return ast;
}


bohExpr* bohAstAllocateExpr(bohAST* pAst)
{
    BOH_ASSERT(pAst);
    return BOH_ARENA_ALLOCATOR_ALLOC(&pAst->epxrMemArena, bohExpr);
}


bohStmt* bohAstAllocateStmt(bohAST* pAst)
{
    BOH_ASSERT(pAst);
    return BOH_ARENA_ALLOCATOR_ALLOC(&pAst->stmtMemArena, bohStmt);
}


bohStmt** bohAstPushStmtPtr(bohAST* pAst, bohStmt* pStmt)
{
    BOH_ASSERT(pAst);
    BOH_ASSERT(pStmt);

    bohStmt** ppStmt = (bohStmt**)bohDynArrayPushBackDummy(&pAst->stmtPtrsStorage);
    *ppStmt = pStmt;
    
    return ppStmt;
}


const bohStmt* bohAstGetStmtByIdx(const bohAST* pAst, size_t index)
{
    BOH_ASSERT(pAst);
    return *BOH_DYN_ARRAY_AT_CONST(bohStmt*, &pAst->stmtPtrsStorage, index);
}


size_t bohAstGetStmtCount(const bohAST* pAst)
{
    BOH_ASSERT(pAst);
    return bohDynArrayGetSize(&pAst->stmtPtrsStorage);
}


size_t bohAstGetMemorySize(const bohAST* pAst)
{
    BOH_ASSERT(pAst);
    
    return bohDynArrayGetMemorySize(&pAst->stmtPtrsStorage) + 
        bohArenaAllocatorGetCapacity(&pAst->epxrMemArena) + 
        bohArenaAllocatorGetCapacity(&pAst->stmtMemArena);
}


bohParser bohParserCreate(const bohTokenStorage *pTokenStorage)
{
    BOH_ASSERT(pTokenStorage);

    bohParser parser;

    parser.pTokenStorage = pTokenStorage;
    parser.currTokenIdx = 0;

    parser.ast = bohAstCreate();

    return parser;
}


void bohParserDestroy(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    pParser->pTokenStorage = NULL;
    pParser->currTokenIdx = 0;

    bohAstDestroy(&pParser->ast);
}


const bohAST* bohParserGetAST(const bohParser* pParser)
{
    BOH_ASSERT(pParser);
    return &pParser->ast;
}


void bohParserParse(bohParser* pParser)
{
    BOH_ASSERT(pParser);
    parsParsStmts(pParser);
}
