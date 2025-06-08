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


static void stmtDefConstr(void* pElement)
{
    BOH_ASSERT(pElement);

    bohStmt* pStmt = (bohStmt*)pElement;
    *pStmt = bohStmtCreate();
}


static void stmtDestr(void* pElement)
{
    BOH_ASSERT(pElement);
    bohStmtDestroy((bohStmt*)pElement);
}


static void stmtCopyFunc(void* pDst, const void* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStmtAssign((bohStmt*)pDst, (bohStmt*)pSrc);
}


static void exprDefConstr(void* pElement)
{
    BOH_ASSERT(pElement);

    bohExpr* pExpr = (bohExpr*)pElement;
    *pExpr = bohExprCreate();
}


static void exprDestr(void* pElement)
{
    BOH_ASSERT(pElement);
    bohExprDestroy((bohExpr*)pElement);
}


static void exprCopyFunc(void* pDst, const void* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohExprAssign((bohExpr*)pDst, (bohExpr*)pSrc);
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
            BOH_ASSERT(false && "Invalid operator type");
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
            BOH_ASSERT(false && "Invalid value expression type");
            break;
    }

    pExpr->type = BOH_VALUE_EXPR_TYPE_NUMBER;
    pExpr->line = 0;
    pExpr->column = 0;
}


bohValueExpr bohValueExprCreate(bohLineNmb line, bohColumnNmb column)
{
    bohValueExpr expr;

    expr.number = bohNumberCreateI64(0);
    expr.type = BOH_VALUE_EXPR_TYPE_NUMBER;
    expr.line = line;
    expr.column = column;

    return expr;
}


bohValueExpr bohValueExprCreateNumber(bohNumber number, bohLineNmb line, bohColumnNmb column)
{
    return bohValueExprCreateNumberNumberPtr(&number, line, column);
}


bohValueExpr bohValueExprCreateNumberNumberPtr(const bohNumber* pNumber, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pNumber);

    bohValueExpr expr = bohValueExprCreate(line, column);
    bohValueExprSetNumberNumberPtr(&expr, pNumber);

    return expr;
}


bohValueExpr bohValueExprCreateStringStringPtr(const bohBoharesString* pString, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pString);

    bohValueExpr expr = bohValueExprCreate(line, column);
    bohValueExprSetStringStringPtr(&expr, pString);

    return expr;
}


bohValueExpr bohValueExprCreateStringStringMove(bohBoharesString* pString, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pString);

    bohValueExpr expr = bohValueExprCreate(line, column);
    bohValueExprSetStringStringMove(&expr, pString);

    return expr;
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


const bohNumber* bohValueExprGetNumber(const bohValueExpr* pExpr)
{
    BOH_ASSERT(bohValueExprIsNumber(pExpr));
    return &pExpr->number;
}


const bohBoharesString* bohValueExprGetString(const bohValueExpr *pExpr)
{
    BOH_ASSERT(bohValueExprIsString(pExpr));
    return &pExpr->string;
}


void bohValueExprSetNumber(bohValueExpr* pExpr, bohNumber number)
{
    bohValueExprSetNumberNumberPtr(pExpr, &number);
}


void bohValueExprSetNumberNumberPtr(bohValueExpr* pExpr, const bohNumber* pNumber)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pNumber);

    const bohLineNmb line = pExpr->line;
    const bohColumnNmb column = pExpr->column;

    bohValueExprDestroy(pExpr);

    bohNumberAssign(&pExpr->number, pNumber);
    pExpr->type = BOH_VALUE_EXPR_TYPE_NUMBER;
    pExpr->line = line;
    pExpr->column = column;
}


void bohValueExprSetStringStringPtr(bohValueExpr* pExpr, const bohBoharesString* pString)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pString);

    const bohLineNmb line = pExpr->line;
    const bohColumnNmb column = pExpr->column;

    bohValueExprDestroy(pExpr);

    bohBoharesStringAssign(&pExpr->string, pString);
    pExpr->type = BOH_VALUE_EXPR_TYPE_STRING;
    pExpr->line = line;
    pExpr->column = column;
}


void bohValueExprSetStringStringMove(bohValueExpr* pExpr, bohBoharesString* pString)
{
    BOH_ASSERT(pExpr);
    BOH_ASSERT(pString);

    const bohLineNmb line = pExpr->line;
    const bohColumnNmb column = pExpr->column;

    bohValueExprDestroy(pExpr);

    bohBoharesStringMove(&pExpr->string, pString);
    pExpr->type = BOH_VALUE_EXPR_TYPE_STRING;
    pExpr->line = line;
    pExpr->column = column;
}


bohValueExpr* bohValueExprAssign(bohValueExpr* pDst, const bohValueExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohValueExprDestroy(pDst);

    pDst->type = pSrc->type;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    switch (pSrc->type) {
        case BOH_VALUE_EXPR_TYPE_NUMBER:
            bohNumberAssign(&pDst->number, &pSrc->number);
            break;
        case BOH_VALUE_EXPR_TYPE_STRING:
            bohBoharesStringAssign(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT(false && "Invalid value expr type");
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
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    switch (pSrc->type) {
        case BOH_VALUE_EXPR_TYPE_NUMBER:
            bohNumberMove(&pDst->number, &pSrc->number);
            break;
        case BOH_VALUE_EXPR_TYPE_STRING:
            bohBoharesStringMove(&pDst->string, &pSrc->string);
            break;
        default:
            BOH_ASSERT(false && "Invalid value expr type");
            break;
    }

    pSrc->type = BOH_VALUE_EXPR_TYPE_NUMBER;
    pSrc->line = 0;
    pSrc->column = 0;

    return pDst;
}


void bohUnaryExprDestroy(bohUnaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    pExpr->exprIdx = BOH_EXPR_IDX_INVALID;
    pExpr->op = BOH_OP_UNKNOWN;
    pExpr->line = 0;
    pExpr->column = 0;
}


bohUnaryExpr bohUnaryExprCreateOpExpr(bohExprOperator op, bohExprIdx exprIdx, bohLineNmb line, bohColumnNmb column)
{
    bohUnaryExpr expr;
    
    expr.exprIdx = exprIdx;
    expr.op = op;
    expr.line = line;
    expr.column = column;

    return expr;
}


bohExprIdx bohUnaryExprGetExprIdx(const bohUnaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->exprIdx;
}


bohExprOperator bohUnaryExprGetOp(const bohUnaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->op;
}


bohUnaryExpr* bohUnaryExprAssign(bohUnaryExpr* pDst, const bohUnaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohUnaryExprDestroy(pDst);

    pDst->exprIdx = pSrc->exprIdx;
    pDst->op = pSrc->op;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


bohUnaryExpr* bohUnaryExprMove(bohUnaryExpr* pDst, bohUnaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohUnaryExprAssign(pDst, pSrc);
    
    pSrc->exprIdx = BOH_EXPR_IDX_INVALID;
    pSrc->op = BOH_OP_UNKNOWN;
    pSrc->line = 0;
    pSrc->column = 0;

    return pDst;
}


void bohBinaryExprDestroy(bohBinaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);

    pExpr->leftExprIdx = BOH_EXPR_IDX_INVALID;
    pExpr->rightExprIdx = BOH_EXPR_IDX_INVALID;
    pExpr->op = BOH_OP_UNKNOWN;
    pExpr->line = 0;
    pExpr->column = 0;
}


bohBinaryExpr bohBinaryExprCreateOpExpr(bohExprOperator op, bohExprIdx leftExprIdx, bohExprIdx rightExprIdx,
    bohLineNmb line, bohColumnNmb column)
{
    bohBinaryExpr expr;

    expr.leftExprIdx = leftExprIdx;
    expr.rightExprIdx = rightExprIdx;
    expr.op = op;
    expr.line = line;
    expr.column = column;

    return expr;
}


bohExprIdx bohBinaryExprGetLeftExprIdx(const bohBinaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->leftExprIdx;
}


bohExprIdx bohBinaryExprGetRightExprIdx(const bohBinaryExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->rightExprIdx;
}


bohExprOperator bohBinaryExprGetOp(const bohBinaryExpr *pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->op;
}


bohBinaryExpr* bohBinaryExprAssign(bohBinaryExpr* pDst, const bohBinaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBinaryExprDestroy(pDst);

    pDst->leftExprIdx = pSrc->leftExprIdx;
    pDst->rightExprIdx = pSrc->rightExprIdx;
    pDst->op = pSrc->op;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


bohBinaryExpr* bohBinaryExprMove(bohBinaryExpr* pDst, bohBinaryExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohBinaryExprAssign(pDst, pSrc);
    
    pSrc->leftExprIdx = BOH_EXPR_IDX_INVALID;
    pSrc->rightExprIdx = BOH_EXPR_IDX_INVALID;
    pSrc->op = BOH_OP_UNKNOWN;
    pSrc->line = 0;
    pSrc->column = 0;

    return pDst;
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
        default:
            BOH_ASSERT(false && "Invalid expression type");
            break;
    }

    pExpr->type = BOH_EXPR_TYPE_VALUE;
    pExpr->selfIdx = BOH_EXPR_IDX_INVALID;
}


bohExpr bohExprCreate(void)
{
    bohExpr expr;

    expr.type = BOH_EXPR_TYPE_VALUE;
    expr.valueExpr = bohValueExprCreateNumber(bohNumberCreateI64(0), 0, 0);
    expr.selfIdx = BOH_EXPR_IDX_INVALID;

    return expr;
}


bohExpr bohExprCreateNumberValueExpr(bohNumber number, bohExprIdx selfIdx, bohLineNmb line, bohColumnNmb column)
{
    return bohExprCreateNumberValueExprPtr(&number, selfIdx, line, column);
}


bohExpr bohExprCreateNumberValueExprPtr(const bohNumber* pNumber, bohExprIdx selfIdx, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pNumber);

    bohExpr expr = bohExprCreate();

    expr.valueExpr = bohValueExprCreateNumberNumberPtr(pNumber, line, column);
    expr.type = BOH_EXPR_TYPE_VALUE;
    expr.selfIdx = selfIdx;

    return expr;
}


bohExpr bohExprCreateStringValueExpr(const bohBoharesString* pString, bohExprIdx selfIdx, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pString);

    bohExpr expr = bohExprCreate();

    expr.valueExpr = bohValueExprCreateStringStringPtr(pString, line, column);
    expr.type = BOH_EXPR_TYPE_VALUE;
    expr.selfIdx = selfIdx;

    return expr;
}


bohExpr bohExprCreateStringValueExprMove(bohBoharesString* pString, bohExprIdx selfIdx, bohLineNmb line, bohColumnNmb column)
{
    BOH_ASSERT(pString);

    bohExpr expr = bohExprCreate();

    expr.valueExpr = bohValueExprCreateStringStringMove(pString, line, column);
    expr.type = BOH_EXPR_TYPE_VALUE;
    expr.selfIdx = selfIdx;

    return expr;
}


bohExpr bohExprCreateUnaryExpr(bohExprOperator op, bohExprIdx selfIdx, bohExprIdx exprIdx, bohLineNmb line, bohColumnNmb column)
{
    bohExpr expr = bohExprCreate();

    expr.unaryExpr = bohUnaryExprCreateOpExpr(op, exprIdx, line, column);
    expr.type = BOH_EXPR_TYPE_UNARY;
    expr.selfIdx = selfIdx;

    return expr;
}


bohExpr bohExprCreateBinaryExpr(bohExprOperator op, bohExprIdx selfIdx, bohExprIdx leftExprIdx, bohExprIdx rightExprIdx, bohLineNmb line, bohColumnNmb column)
{
    bohExpr expr = bohExprCreate();

    expr.binaryExpr = bohBinaryExprCreateOpExpr(op, leftExprIdx, rightExprIdx, line, column);
    expr.type = BOH_EXPR_TYPE_BINARY;
    expr.selfIdx = selfIdx;

    return expr;
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


bohExprIdx bohExprGetStorageIdx(const bohExpr* pExpr)
{
    BOH_ASSERT(pExpr);
    return pExpr->selfIdx;
}


bohExpr* bohExprAssign(bohExpr* pDst, const bohExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohExprDestroy(pDst);

    pDst->type = pSrc->type;
    pDst->selfIdx = pSrc->selfIdx;

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
        default:
            BOH_ASSERT(false && "Invalid expression type");
            break;
    }

    return pDst;
}


bohExpr* bohExprMove(bohExpr* pDst, bohExpr* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohExprDestroy(pDst);

    pDst->type = pSrc->type;
    pDst->selfIdx = pSrc->selfIdx;

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
        default:
            BOH_ASSERT(false && "Invalid expression type");
            break;
    }

    pSrc->type = BOH_EXPR_TYPE_VALUE;
    pSrc->selfIdx = BOH_EXPR_IDX_INVALID;

    return pDst;
}


void bohRawExprStmtDestroy(bohRawExprStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    pStmt->exprIdx = BOH_EXPR_IDX_INVALID;
    pStmt->line = 0;
    pStmt->column = 0;
}


bohRawExprStmt bohRawExprStmtCreateExprIdx(bohExprIdx exprIdx, bohLineNmb line, bohColumnNmb column)
{
    bohRawExprStmt stmt;
    stmt.exprIdx = exprIdx;
    stmt.line = line;
    stmt.column = column;

    return stmt;
}


bohExprIdx bohRawExprStmtGetExprIdx(const bohRawExprStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->exprIdx;
}


bohRawExprStmt* bohRawExprStmtAssign(bohRawExprStmt* pDst, const bohRawExprStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohRawExprStmtDestroy(pDst);
    
    pDst->exprIdx = pSrc->exprIdx;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


bohRawExprStmt* bohRawExprStmtMove(bohRawExprStmt* pDst, bohRawExprStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohRawExprStmtAssign(pDst, pSrc);

    pSrc->exprIdx = BOH_EXPR_IDX_INVALID;
    pSrc->line = 0;
    pSrc->column = 0;

    return pDst;
}


void bohPrintStmtDestroy(bohPrintStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    pStmt->argStmtIdx = BOH_STMT_IDX_INVALID;
    pStmt->line = 0;
    pStmt->column = 0;
}


bohPrintStmt bohPrintStmtCreateStmtIdx(bohStmtIdx argStmtIdx, bohLineNmb line, bohColumnNmb column)
{
    bohPrintStmt stmt;
    stmt.argStmtIdx = argStmtIdx;
    stmt.line = line;
    stmt.column = column;

    return stmt;
}


bohStmtIdx bohPrintStmtGetStmtIdx(const bohPrintStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->argStmtIdx;
}


bohPrintStmt* bohPrintStmtAssign(bohPrintStmt* pDst, const bohPrintStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohPrintStmtDestroy(pDst);

    pDst->argStmtIdx = pSrc->argStmtIdx;
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


bohPrintStmt* bohPrintStmtMove(bohPrintStmt* pDst, bohPrintStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohPrintStmtAssign(pDst, pSrc);

    pSrc->argStmtIdx = BOH_STMT_IDX_INVALID;
    pSrc->line = 0;
    pSrc->column = 0;

    return pDst;
}


void bohStmtDestroy(bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);

    switch (pStmt->type) {
        case BOH_STMT_TYPE_EMPTY:
            break;
        case BOH_STMT_TYPE_RAW_EXPR:
            bohRawExprStmtDestroy(&pStmt->rawExpr);
            break;
        case BOH_STMT_TYPE_PRINT:
            bohPrintStmtDestroy(&pStmt->printStmt);
            break;
        default:
            BOH_ASSERT(false && "Invalid statement type");
            break;
    }

    pStmt->type = BOH_STMT_TYPE_EMPTY;
    pStmt->selfIdx = BOH_STMT_IDX_INVALID;
}


bohStmt bohStmtCreate(void)
{
    bohStmt stmt;

    memset(&stmt, 0, sizeof(bohStmt));

    stmt.type = BOH_STMT_TYPE_EMPTY;
    stmt.selfIdx = BOH_STMT_IDX_INVALID;

    return stmt;
}


bohStmt bohStmtCreateRawExpr(bohStmtIdx selfIdx, bohExprIdx exprIdx, bohLineNmb line, bohColumnNmb column)
{
    bohStmt stmt = bohStmtCreate();

    stmt.type = BOH_STMT_TYPE_RAW_EXPR;
    stmt.rawExpr = bohRawExprStmtCreateExprIdx(exprIdx, line, column);
    stmt.selfIdx = selfIdx;

    return stmt;
}


bohStmt bohStmtCreatePrint(bohStmtIdx selfIdx, bohStmtIdx argStmtIdx, bohLineNmb line, bohColumnNmb column)
{
    bohStmt stmt = bohStmtCreate();

    stmt.type = BOH_STMT_TYPE_PRINT;
    stmt.printStmt = bohPrintStmtCreateStmtIdx(argStmtIdx, line, column);
    stmt.selfIdx = selfIdx;

    return stmt;
}


bohStmtType bohStmtGetType(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type;
}


bool bohStmtIsRawExpr(const bohStmt* pStmt)
{
    BOH_ASSERT(pStmt);
    return pStmt->type == BOH_STMT_TYPE_RAW_EXPR;
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


const bohRawExprStmt* bohStmtGetRawExpr(const bohStmt* pStmt)
{
    BOH_ASSERT(bohStmtIsRawExpr(pStmt));
    return &pStmt->rawExpr;
}


const bohPrintStmt* bohStmtGetPrint(const bohStmt* pStmt)
{
    BOH_ASSERT(bohStmtIsPrint(pStmt));
    return &pStmt->printStmt;
}


bohStmt* bohStmtAssign(bohStmt* pDst, const bohStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStmtDestroy(pDst);

    pDst->type = pSrc->type;
    pDst->selfIdx = pSrc->selfIdx;

    switch (pSrc->type) {
        case BOH_STMT_TYPE_EMPTY:
            break;
        case BOH_STMT_TYPE_RAW_EXPR:
            bohRawExprStmtAssign(&pDst->rawExpr, &pSrc->rawExpr);
            break;
        case BOH_STMT_TYPE_PRINT:
            bohPrintStmtAssign(&pDst->printStmt, &pSrc->printStmt);
            break;
        default:
            BOH_ASSERT(false && "Invalid statement type");
            break;
    }

    return pDst;
}


bohStmt* bohStmtMove(bohStmt* pDst, bohStmt* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStmtDestroy(pDst);

    pDst->type = pSrc->type;
    pDst->selfIdx = pSrc->selfIdx;

    switch (pSrc->type) {
        case BOH_STMT_TYPE_EMPTY:
            break;
        case BOH_STMT_TYPE_RAW_EXPR:
            bohRawExprStmtMove(&pDst->rawExpr, &pSrc->rawExpr);
            break;
        case BOH_STMT_TYPE_PRINT:
            bohPrintStmtMove(&pDst->printStmt, &pSrc->printStmt);
            break;
        default:
            BOH_ASSERT(false && "Invalid statement type");
            break;
    }

    pSrc->type = BOH_STMT_TYPE_EMPTY;
    pSrc->selfIdx = BOH_STMT_IDX_INVALID;

    return pDst;
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
    return bohDynArrayAtConst(pParser->pTokenStorage, pParser->currTokenIdx);
}


static const bohToken* parsPeekPrevToken(const bohParser* pParser)
{
    BOH_ASSERT(pParser);
    BOH_ASSERT(pParser->currTokenIdx > 0);

    const size_t prevTokenIdx = pParser->currTokenIdx - 1;
    BOH_ASSERT(prevTokenIdx < bohDynArrayGetSize(pParser->pTokenStorage));

    return bohDynArrayAtConst(pParser->pTokenStorage, prevTokenIdx);
}


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


static bohExpr parsParsExpr(bohParser* pParser);


// <primary> = <integer> | <float> | <string> | '(' <expr> ')' 
static bohExpr parsParsPrimary(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_TRUE)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        *pPrimaryExpr = bohExprCreateNumberValueExpr(bohNumberCreateI64(1), bohExprGetStorageIdx(pPrimaryExpr),
            parsPeekPrevToken(pParser)->line, parsPeekPrevToken(pParser)->column);
        
        return *pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FALSE)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        *pPrimaryExpr = bohExprCreateNumberValueExpr(bohNumberCreateI64(0), bohExprGetStorageIdx(pPrimaryExpr),
            parsPeekPrevToken(pParser)->line, parsPeekPrevToken(pParser)->column);

        return *pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_INTEGER)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        const int64_t value = _atoi64(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        *pPrimaryExpr = bohExprCreateNumberValueExpr(bohNumberCreateI64(value), bohExprGetStorageIdx(pPrimaryExpr),
            parsPeekPrevToken(pParser)->line, parsPeekPrevToken(pParser)->column);

        return *pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FLOAT)) {
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        const double value = atof(bohStringViewGetData(&parsPeekPrevToken(pParser)->lexeme));
        *pPrimaryExpr = bohExprCreateNumberValueExpr(bohNumberCreateF64(value), bohExprGetStorageIdx(pPrimaryExpr),
            parsPeekPrevToken(pParser)->line, parsPeekPrevToken(pParser)->column);
        
        return *pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_STRING)) {
        bohBoharesString lexeme = bohBoharesStringCreateStringViewStringViewPtr(&parsPeekPrevToken(pParser)->lexeme);
        bohBoharesString unescapedLexeme = parsGetUnescapedString(&lexeme);

        bohBoharesStringDestroy(&lexeme);
        
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);

        *pPrimaryExpr = bohExprCreateStringValueExprMove(&unescapedLexeme, bohExprGetStorageIdx(pPrimaryExpr), 
            parsPeekPrevToken(pParser)->line, parsPeekPrevToken(pParser)->column);

        return *pPrimaryExpr;
    } else if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LPAREN)) {
        const uint32_t line = parsPeekCurrToken(pParser)->line;
        const uint32_t column = parsPeekCurrToken(pParser)->column;

        bohExpr expr = parsParsExpr(pParser);
        BOH_PARSER_EXPECT(parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RPAREN), line, column, "missed closing \')\'");
        
        return expr;
    }

    {
        const bohToken* pCurrToken = parsPeekCurrToken(pParser);
        BOH_PARSER_EXPECT(false, pCurrToken->line, pCurrToken->column, "invalid primary token: %.*s",
            bohStringViewGetSize(&pCurrToken->lexeme), bohStringViewGetData(&pCurrToken->lexeme));
    
        bohExpr* pPrimaryExpr = bohAstAllocateExpr(&pParser->ast);
    
        *pPrimaryExpr = bohExprCreateNumberValueExpr(bohNumberCreateI64(0), bohExprGetStorageIdx(pPrimaryExpr),
            pCurrToken->line, pCurrToken->column);

        return *pPrimaryExpr;
    }
}


// <unary> = ('+' | '-' | '~' | '!') <unary> | <primary>
static bohExpr parsParsUnary(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    if (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PLUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_NOT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_NOT)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);

        bohExpr operandExpr = parsParsUnary(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, "unknown unary operator: %.*s", 
            bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));
        
        bohExpr* pUnaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pUnaryExpr = bohExprCreateUnaryExpr(op, pUnaryExpr->selfIdx, operandExpr.selfIdx, pOperatorToken->line, pOperatorToken->column);

        return *pUnaryExpr;
    }

    return parsParsPrimary(pParser);
}


// <multiplication> = <unary> (('*' | '/' | '%') <unary>)*
static bohExpr parsParsMultiplication(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsUnary(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MULT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_DIV) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MOD)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsUnary(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown term operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <addition> = <multiplication> (('+' | '-') <multiplication>)*
static bohExpr parsParsAddition(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsMultiplication(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PLUS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_MINUS)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsMultiplication(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <bitwise_shift> = <addition> (('<<' | '>>') <addition>)*
static bohExpr parsParsBitwiseShift(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsAddition(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_LSHIFT) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_RSHIFT)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsAddition(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <comparison> = <bitwise_shift> (('>' | '>=' | '<' | '<=') <bitwise_shift>)*
static bohExpr parsParsComparison(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsBitwiseShift(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_GREATER) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LESS) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_GEQUAL) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LEQUAL)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsBitwiseShift(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <equality> = <comparison> (('!=' | '==') <comparison>)*
static bohExpr parsParsEquality(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsComparison(pParser);

    while (
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_NOT_EQUAL) ||
        parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_EQUAL)
    ) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsComparison(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <bitwise_and> = <equality> (('&') <equality>)*
static bohExpr parsParsBitwiseAnd(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsEquality(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_AND)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsEquality(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <bitwise_xor> = <bitwise_and> (('^') <bitwise_and>)*
static bohExpr parsParsBitwiseXor(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsBitwiseAnd(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_XOR)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsBitwiseAnd(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <bitwise_or> = <bitwise_xor> (('|') <bitwise_xor>)*
static bohExpr parsParsBitwiseOr(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsBitwiseXor(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_BITWISE_OR)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsBitwiseXor(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <and> = <bitwise_or> (('and' | '&&') <bitwise_or>)*
static bohExpr parsParsAnd(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsBitwiseOr(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_AND)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsBitwiseOr(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


// <or> = <and> (('or' | '||') <and>)*
static bohExpr parsParsOr(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    bohExpr leftExpr = parsParsAnd(pParser);

    while (parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_OR)) {
        const bohToken* pOperatorToken = parsPeekPrevToken(pParser);
        const bohExpr rightExpr = parsParsAnd(pParser);

        const bohExprOperator op = parsTokenTypeToExprOperator(pOperatorToken->type);
        BOH_PARSER_EXPECT(op != BOH_OP_UNKNOWN, pOperatorToken->line, pOperatorToken->column, 
            "unknown expr operator: %.*s", bohStringViewGetSize(&pOperatorToken->lexeme), bohStringViewGetData(&pOperatorToken->lexeme));

        bohExpr* pBinaryExpr = bohAstAllocateExpr(&pParser->ast);
        *pBinaryExpr = bohExprCreateBinaryExpr(op, pBinaryExpr->selfIdx, leftExpr.selfIdx, rightExpr.selfIdx, 
            pOperatorToken->line, pOperatorToken->column);

        leftExpr = *pBinaryExpr;
    }

    return leftExpr;
}


static bohExpr parsParsExpr(bohParser* pParser)
{
    return parsParsOr(pParser);
}


static bohStmt parsParsNextStmt(bohParser* pParser);


// <raw_expr_stmt> = <expr>
static bohStmt parsParsRawExprStmt(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const bohToken* pCurrToken = parsPeekCurrToken(pParser);
    
    // const bool isRawExprStmt = parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_STRING) ||
    //     parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_INTEGER) ||
    //     parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_FLOAT) ||
    //     parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_LPAREN);
    // BOH_ASSERT(isRawExprStmt);

    const bohExpr expr = parsParsExpr(pParser);

    /////////////////
    // NEED TO CHECK AND THINK ABOUT CHECK_COND
    // parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_RPAREN);
    /////////////////

    bohStmt* pRawExprStmt = bohAstAllocateStmt(&pParser->ast);
    *pRawExprStmt = bohStmtCreateRawExpr(pRawExprStmt->selfIdx, expr.selfIdx, pCurrToken->line, pCurrToken->column);

    return *pRawExprStmt;
}


// <print_stmt> = "print" <stmt>
static bohStmt parsParsPrintStmt(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const bohToken* pCurrToken = parsPeekCurrToken(pParser);
    
    const bool isPrintStmt = parsIsCurrTokenMatch(pParser, BOH_TOKEN_TYPE_PRINT);
    BOH_ASSERT(isPrintStmt);

    bohStmt* pPrintStmt = bohAstAllocateStmt(&pParser->ast);

    const bohStmt argStmt = parsParsNextStmt(pParser);
    *pPrintStmt = bohStmtCreatePrint(pPrintStmt->selfIdx, argStmt.selfIdx, pCurrToken->line, pCurrToken->column);

    return *pPrintStmt;
}


static bohStmt parsParsNextStmt(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const bohToken* pCurrToken = parsPeekCurrToken(pParser);

    switch (pCurrToken->type) {
        case BOH_TOKEN_TYPE_LPAREN:
        case BOH_TOKEN_TYPE_STRING:
        case BOH_TOKEN_TYPE_INTEGER:
        case BOH_TOKEN_TYPE_FLOAT:
            return parsParsRawExprStmt(pParser);
        case BOH_TOKEN_TYPE_PRINT:
            return parsParsPrintStmt(pParser);
        default:
            BOH_ASSERT(false && "Invalid token type");
            return bohStmtCreate();
    }
}


static void parsParsStmts(bohParser* pParser)
{
    BOH_ASSERT(pParser);

    const bohTokenStorage* pTokenStorage = pParser->pTokenStorage;
    const size_t tokensCount = bohDynArrayGetSize(pTokenStorage);
    
    while(pParser->currTokenIdx < tokensCount) {
        parsParsNextStmt(pParser);
    }
}


void bohAstDestroy(bohAST* pAST)
{
    BOH_ASSERT(pAST);
    bohDynArrayDestroy(&pAST->stmts);
    bohDynArrayDestroy(&pAST->exprs);
}


bohAST bohAstCreate(
    const bohStmtStorageCreateInfo* pStmtStorageCreateInfo, 
    const bohExprStorageCreateInfo* pExprStorageCreateInfo
) {
    BOH_ASSERT(pStmtStorageCreateInfo);
    BOH_ASSERT(pStmtStorageCreateInfo->pStmtDefConstr);
    BOH_ASSERT(pStmtStorageCreateInfo->pStmtDestr);
    BOH_ASSERT(pStmtStorageCreateInfo->pStmtCopyFunc);

    BOH_ASSERT(pExprStorageCreateInfo);
    BOH_ASSERT(pExprStorageCreateInfo->pExprDefConstr);
    BOH_ASSERT(pExprStorageCreateInfo->pExprDestr);
    BOH_ASSERT(pExprStorageCreateInfo->pExprCopyFunc);

    bohAST ast;

    ast.stmts = BOH_DYN_ARRAY_CREATE(
        bohStmt, 
        pStmtStorageCreateInfo->pStmtDefConstr, 
        pStmtStorageCreateInfo->pStmtDestr, 
        pStmtStorageCreateInfo->pStmtCopyFunc
    );

    ast.exprs = BOH_DYN_ARRAY_CREATE(
        bohExpr, 
        pExprStorageCreateInfo->pExprDefConstr, 
        pExprStorageCreateInfo->pExprDestr, 
        pExprStorageCreateInfo->pExprCopyFunc
    );

    return ast;
}


bohExpr* bohAstAllocateExpr(bohAST* pAst)
{
    BOH_ASSERT(pAst);

    bohExpr* pExpr = bohDynArrayPushBackDummy(&pAst->exprs);
    BOH_ASSERT(pExpr);

    pExpr->selfIdx = bohDynArrayGetSize(&pAst->exprs) - 1;

    return pExpr;
}


bohStmt* bohAstAllocateStmt(bohAST* pAst)
{
    BOH_ASSERT(pAst);

    bohStmt* pStmt = bohDynArrayPushBackDummy(&pAst->stmts);
    BOH_ASSERT(pStmt);

    pStmt->selfIdx = bohDynArrayGetSize(&pAst->stmts) - 1;

    return pStmt;
}


bohStmtStorage* bohAstGetStmts(bohAST* pAst)
{
    BOH_ASSERT(pAst);
    return &pAst->stmts;
}


const bohStmtStorage* bohAstGetStmtsConst(const bohAST* pAst)
{
    BOH_ASSERT(pAst);
    return &pAst->stmts;
}


const bohStmt* bohAstGetStmtByIdx(const bohAST* pAst, size_t index)
{
    BOH_ASSERT(pAst);
    return bohDynArrayAtConst(&pAst->stmts, index);
}


bohExprStorage* bohAstGetExprs(bohAST* pAst)
{
    BOH_ASSERT(pAst);
    return &pAst->exprs;
}


const bohExprStorage* bohAstGetExprsConst(const bohAST* pAst)
{
    BOH_ASSERT(pAst);
    return &pAst->exprs;
}


const bohExpr* bohAstGetExprByIdx(const bohAST* pAst, size_t index)
{
    BOH_ASSERT(pAst);
    return bohDynArrayAtConst(&pAst->exprs, index);
}


bool bohAstIsEmpty(const bohAST *pAst)
{
    BOH_ASSERT(pAst);
    return bohDynArrayIsEmpty(&pAst->stmts);
}


bohParser bohParserCreate(const bohTokenStorage *pTokenStorage)
{
    BOH_ASSERT(pTokenStorage);

    bohParser parser;

    parser.pTokenStorage = pTokenStorage;
    parser.currTokenIdx = 0;

    bohStmtStorageCreateInfo stmtStorageCreateInfo;
    stmtStorageCreateInfo.pStmtDefConstr = stmtDefConstr;
    stmtStorageCreateInfo.pStmtDestr = stmtDestr;
    stmtStorageCreateInfo.pStmtCopyFunc = stmtCopyFunc;

    bohExprStorageCreateInfo exprStorageCreateInfo;
    exprStorageCreateInfo.pExprDefConstr = exprDefConstr;
    exprStorageCreateInfo.pExprDestr = exprDestr;
    exprStorageCreateInfo.pExprCopyFunc = exprCopyFunc;

    parser.ast = bohAstCreate(&stmtStorageCreateInfo, &exprStorageCreateInfo);

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
