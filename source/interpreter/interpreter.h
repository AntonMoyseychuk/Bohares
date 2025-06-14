#pragma once

#include "types.h"

#include "parser/parser.h"


typedef enum RawExprStmtInterpResultType
{
    BOH_RAW_EXPR_INTERP_RES_TYPE_NUMBER,
    BOH_RAW_EXPR_INTERP_RES_TYPE_STRING
} bohRawExprStmtInterpResultType;

const char* bohRawExprStmtInterpResultTypeToStr(bohRawExprStmtInterpResultType type);


typedef struct RawExprStmtInterpResult
{
    bohRawExprStmtInterpResultType type;
    
    union {
        bohBoharesString string;
        bohNumber number;
    };
} bohRawExprStmtInterpResult;


bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreate(void);
void bohRawExprStmtInterpResultDestroy(bohRawExprStmtInterpResult* pResult);

bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringStringViewPtr(bohRawExprStmtInterpResult* pResult, const bohStringView* pStrView);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringStringView(bohRawExprStmtInterpResult* pResult, bohStringView strView);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringCStr(bohRawExprStmtInterpResult* pResult, const char* pCStr);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetString(bohRawExprStmtInterpResult* pResult, const bohString* pString);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringViewStringViewPtr(bohRawExprStmtInterpResult* pResult, const bohStringView* pStrView);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetStringViewStringView(bohRawExprStmtInterpResult* pResult, bohStringView strView);

bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumberPtr(bohRawExprStmtInterpResult* pResult, const bohNumber* pNumber);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumber(bohRawExprStmtInterpResult* pResult, bohNumber number);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumberI64(bohRawExprStmtInterpResult* pResult, int64_t value);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultSetNumberF64(bohRawExprStmtInterpResult* pResult, double value);

bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateString(const bohString* pString);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringCStr(const char* pCStr);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringStringViewPtr(const bohStringView* pStrView);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringStringView(bohStringView strView);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringBoharesStringRVal(bohBoharesString string);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateStringViewStringView(bohStringView strView);

bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumberPtr(const bohNumber* pNumber);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumber(bohNumber number);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumberI64(int64_t value);
bohRawExprStmtInterpResult bohRawExprStmtInterpResultCreateNumberF64(double value);

bool bohRawExprStmtInterpResultIsNumber(const bohRawExprStmtInterpResult* pResult);
bool bohRawExprStmtInterpResultIsNumberI64(const bohRawExprStmtInterpResult* pResult);
bool bohRawExprStmtInterpResultIsNumberF64(const bohRawExprStmtInterpResult* pResult);
bool bohRawExprStmtInterpResultIsString(const bohRawExprStmtInterpResult* pResult);
bool bohRawExprStmtInterpResultIsStringStringView(const bohRawExprStmtInterpResult* pResult);
bool bohRawExprStmtInterpResultIsStringString(const bohRawExprStmtInterpResult* pResult);

const bohNumber* bohRawExprStmtInterpResultGetNumber(const bohRawExprStmtInterpResult* pResult);
int64_t bohRawExprStmtInterpResultGetNumberI64(const bohRawExprStmtInterpResult* pResult);
double bohRawExprStmtInterpResultGetNumberF64(const bohRawExprStmtInterpResult* pResult);

bool bohRawExprStmtInterpResultToBool(const bohRawExprStmtInterpResult* pResult);

const bohBoharesString* bohRawExprStmtInterpResultGetString(const bohRawExprStmtInterpResult* pResult);
const bohString* bohRawExprStmtInterpResultGetStringString(const bohRawExprStmtInterpResult* pResult);
const bohStringView* bohRawExprStmtInterpResultGetStringStringView(const bohRawExprStmtInterpResult* pResult);

bohRawExprStmtInterpResult* bohRawExprStmtInterpResultMove(bohRawExprStmtInterpResult* pDst, bohRawExprStmtInterpResult* pSrc);
bohRawExprStmtInterpResult* bohRawExprStmtInterpResultAssing(bohRawExprStmtInterpResult* pDst, const bohRawExprStmtInterpResult* pSrc);


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
    union {
        bohRawExprStmtInterpResult rawExprStmtInterpResult;
        bohPrintStmtInterpResult printStmtInterpResult;
        bohIfStmtInterpResult ifStmtInterpResult;
    };

    bohStmtInterpResultType type;
    bohStmtIdx lastInterpretedStmtIdx;
} bohStmtInterpResult;


bohStmtInterpResult bohStmtInterpResultCreate(void);
void bohStmtInterpResultDestroy(bohStmtInterpResult* pResult);

bool bohStmtInterpResultIsRawExprStmt(const bohStmtInterpResult* pResult);
bool bohStmtInterpResultIsPrintStmt(const bohStmtInterpResult* pResult);
bool bohStmtInterpResultIsIfStmt(const bohStmtInterpResult* pResult);

const bohRawExprStmtInterpResult* bohStmtInterpResultGetRawExprStmtResult(const bohStmtInterpResult* pResult);
const bohPrintStmtInterpResult* bohStmtInterpResultGetPrintStmtResult(const bohStmtInterpResult* pResult);
const bohIfStmtInterpResult* bohStmtInterpResultGetIfStmtResult(const bohStmtInterpResult* pResult);

bohStmtInterpResult bohStmtInterpResultCreateRawExprResultMove(bohRawExprStmtInterpResult* pResult, bohStmtIdx idx);
bohStmtInterpResult bohStmtInterpResultCreatePrintStmtMove(bohPrintStmtInterpResult* pResult, bohStmtIdx idx);
bohStmtInterpResult bohStmtInterpResultCreateIfStmtMove(bohIfStmtInterpResult* pResult, bohStmtIdx idx);


typedef struct AST bohAST;

typedef struct Interpreter
{
    const bohAST* pAst;
} bohInterpreter;


bohInterpreter bohInterpCreate(const bohAST* pAst);
void bohInterpDestroy(bohInterpreter* pInterp);

void bohInterpInterpret(bohInterpreter* pInterp);
