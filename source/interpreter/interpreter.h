#pragma once

#include "types.h"

#include "parser/parser.h"


typedef enum ExprStmtInterpResultType
{
    BOH_EXPR_INTERP_RES_TYPE_NUMBER,
    BOH_EXPR_INTERP_RES_TYPE_STRING
} bohExprInterpResultType;

const char* bohExprInterpResultTypeToStr(bohExprInterpResultType type);


typedef struct ExprInterpResult
{
    bohExprInterpResultType type;
    
    union {
        bohBoharesString string;
        bohNumber number;
    };
} bohExprInterpResult;


bohExprInterpResult bohExprInterpResultCreate(void);
void bohExprInterpResultDestroy(bohExprInterpResult* pResult);

bohExprInterpResult* bohExprInterpResultSetStringStringViewPtr(bohExprInterpResult* pResult, const bohStringView* pStrView);
bohExprInterpResult* bohExprInterpResultSetStringStringView(bohExprInterpResult* pResult, bohStringView strView);
bohExprInterpResult* bohExprInterpResultSetStringCStr(bohExprInterpResult* pResult, const char* pCStr);
bohExprInterpResult* bohExprInterpResultSetString(bohExprInterpResult* pResult, const bohString* pString);
bohExprInterpResult* bohExprInterpResultSetStringViewStringViewPtr(bohExprInterpResult* pResult, const bohStringView* pStrView);
bohExprInterpResult* bohExprInterpResultSetStringViewStringView(bohExprInterpResult* pResult, bohStringView strView);

bohExprInterpResult* bohExprInterpResultSetNumberPtr(bohExprInterpResult* pResult, const bohNumber* pNumber);
bohExprInterpResult* bohExprInterpResultSetNumber(bohExprInterpResult* pResult, bohNumber number);
bohExprInterpResult* bohExprInterpResultSetNumberI64(bohExprInterpResult* pResult, int64_t value);
bohExprInterpResult* bohExprInterpResultSetNumberF64(bohExprInterpResult* pResult, double value);

bohExprInterpResult bohExprInterpResultCreateString(const bohString* pString);
bohExprInterpResult bohExprInterpResultCreateStringCStr(const char* pCStr);
bohExprInterpResult bohExprInterpResultCreateStringStringViewPtr(const bohStringView* pStrView);
bohExprInterpResult bohExprInterpResultCreateStringStringView(bohStringView strView);
bohExprInterpResult bohExprInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString);
bohExprInterpResult bohExprInterpResultCreateStringBoharesStringRVal(bohBoharesString string);
bohExprInterpResult bohExprInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString);
bohExprInterpResult bohExprInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView);
bohExprInterpResult bohExprInterpResultCreateStringViewStringView(bohStringView strView);

bohExprInterpResult bohExprInterpResultCreateNumberPtr(const bohNumber* pNumber);
bohExprInterpResult bohExprInterpResultCreateNumber(bohNumber number);
bohExprInterpResult bohExprInterpResultCreateNumberI64(int64_t value);
bohExprInterpResult bohExprInterpResultCreateNumberF64(double value);

bool bohExprInterpResultIsNumber(const bohExprInterpResult* pResult);
bool bohExprInterpResultIsNumberI64(const bohExprInterpResult* pResult);
bool bohExprInterpResultIsNumberF64(const bohExprInterpResult* pResult);
bool bohExprInterpResultIsString(const bohExprInterpResult* pResult);
bool bohExprInterpResultIsStringStringView(const bohExprInterpResult* pResult);
bool bohExprInterpResultIsStringString(const bohExprInterpResult* pResult);

const bohNumber* bohExprInterpResultGetNumber(const bohExprInterpResult* pResult);
int64_t bohExprInterpResultGetNumberI64(const bohExprInterpResult* pResult);
double bohExprInterpResultGetNumberF64(const bohExprInterpResult* pResult);

bool bohExprInterpResultToBool(const bohExprInterpResult* pResult);

const bohBoharesString* bohExprInterpResultGetString(const bohExprInterpResult* pResult);
const bohString* bohExprInterpResultGetStringString(const bohExprInterpResult* pResult);
const bohStringView* bohExprInterpResultGetStringStringView(const bohExprInterpResult* pResult);

bohExprInterpResult* bohExprInterpResultMove(bohExprInterpResult* pDst, bohExprInterpResult* pSrc);
bohExprInterpResult* bohExprInterpResultAssing(bohExprInterpResult* pDst, const bohExprInterpResult* pSrc);


typedef struct PrintStmtInterpResult
{
} bohPrintStmtInterpResult;


bohPrintStmtInterpResult bohPrintStmtInterpResultCreate(void);
void bohPrintStmtInterpResultDestroy(bohPrintStmtInterpResult* pResult);

bohPrintStmtInterpResult* bohPrintStmtInterpResultMove(bohPrintStmtInterpResult* pDst, bohPrintStmtInterpResult* pSrc);
bohPrintStmtInterpResult* bohPrintStmtInterpResultAssign(bohPrintStmtInterpResult* pDst, bohPrintStmtInterpResult* pSrc);


typedef struct IfStmtInterpResult
{
} bohIfStmtInterpResult;


bohIfStmtInterpResult bohIfStmtInterpResultCreate(void);
void bohIfStmtInterpResultDestroy(bohIfStmtInterpResult* pResult);

bohIfStmtInterpResult* bohIfStmtInterpResultMove(bohIfStmtInterpResult* pDst, bohIfStmtInterpResult* pSrc);
bohIfStmtInterpResult* bohIfStmtInterpResultAssign(bohIfStmtInterpResult* pDst, bohIfStmtInterpResult* pSrc);


typedef enum StmtInterpResultType
{
    BOH_INTERP_RES_TYPE_RAW_EXPR,
    BOH_INTERP_RES_TYPE_PRINT,
    BOH_INTERP_RES_TYPE_IF,
} bohStmtInterpResultType;


typedef struct StmtInterpResult
{
    bohStmtInterpResultType type;
    union {
        bohExprInterpResult exprInterpResult;
        bohPrintStmtInterpResult printStmtInterpResult;
        bohIfStmtInterpResult ifStmtInterpResult;
    };
} bohStmtInterpResult;


void bohStmtInterpResultDestroy(bohStmtInterpResult* pResult);

bohStmtInterpResult bohStmtInterpResultCreateExprResultMove(bohExprInterpResult* pResult);
bohStmtInterpResult bohStmtInterpResultCreatePrintStmtMove(bohPrintStmtInterpResult* pResult);
bohStmtInterpResult bohStmtInterpResultCreateIfStmtMove(bohIfStmtInterpResult* pResult);

bool bohStmtInterpResultIsExpr(const bohStmtInterpResult* pResult);
bool bohStmtInterpResultIsPrintStmt(const bohStmtInterpResult* pResult);
bool bohStmtInterpResultIsIfStmt(const bohStmtInterpResult* pResult);

const bohExprInterpResult* bohStmtInterpResultGetExprResult(const bohStmtInterpResult* pResult);
const bohPrintStmtInterpResult* bohStmtInterpResultGetPrintStmtResult(const bohStmtInterpResult* pResult);
const bohIfStmtInterpResult* bohStmtInterpResultGetIfStmtResult(const bohStmtInterpResult* pResult);


typedef struct AST bohAST;

typedef struct Interpreter
{
    const bohAST* pAst;
} bohInterpreter;


bohInterpreter bohInterpCreate(const bohAST* pAst);
void bohInterpDestroy(bohInterpreter* pInterp);

void bohInterpInterpret(bohInterpreter* pInterp);
