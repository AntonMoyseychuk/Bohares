#pragma once

#include "types.h"


typedef enum InterpResultType
{
    BOH_INTERP_RES_TYPE_NUMBER,
    BOH_INTERP_RES_TYPE_STRING
} bohInterpResultType;


typedef union InterpResult
{
    bohInterpResultType type;
    
    union {
        bohBoharesString string;
        bohNumber number;
    };
} bohInterpResult;


bohInterpResult bohInterpResultCreateString(const bohString* pString);
bohInterpResult bohInterpResultCreateStringCStr(const char* pCStr);
bohInterpResult bohInterpResultCreateStringStringView(bohStringView strView);
bohInterpResult bohInterpResultCreateStringStringViewPtr(const bohStringView* pStrView);
bohInterpResult bohInterpResultCreateStringViewStringView(bohStringView strView);
bohInterpResult bohInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView);
bohInterpResult bohInterpResultCreateNumber(bohNumber number);
bohInterpResult bohInterpResultCreateNumberPtr(const bohNumber* pNumber);

void bohInterpResultDestroy(bohInterpResult* pResult);

bool bohInterpResultIsNumber(const bohInterpResult* pResult);
bool bohInterpResultIsNumberI64(const bohInterpResult* pResult);
bool bohInterpResultIsNumberF64(const bohInterpResult* pResult);
bool bohInterpResultIsString(const bohInterpResult* pResult);
bool bohInterpResultIsStringStringView(const bohInterpResult* pResult);
bool bohInterpResultIsStringString(const bohInterpResult* pResult);

const bohNumber*        bohInterpResultGetNumber(const bohInterpResult* pResult);
int64_t                 bohInterpResultGetNumberI64(const bohInterpResult* pResult);
double                  bohInterpResultGetNumberF64(const bohInterpResult* pResult);
const bohBoharesString* bohInterpResultGetString(const bohInterpResult* pResult);
const bohString*        bohInterpResultGetStringString(const bohInterpResult* pResult);
const bohStringView*    bohInterpResultGetStringStringView(const bohInterpResult* pResult);

bohInterpResult* bohInterpResultSetString(bohInterpResult* pResult, const bohString* pString);
bohInterpResult* bohInterpResultSetStringCStr(bohInterpResult* pResult, const char* pCStr);
bohInterpResult* bohInterpResultSetStringStringView(bohInterpResult* pResult, bohStringView strView);
bohInterpResult* bohInterpResultSetStringStringViewPtr(bohInterpResult* pResult, const bohStringView* pStrView);
bohInterpResult* bohInterpResultSetStringViewStringView(bohInterpResult* pResult, bohStringView strView);
bohInterpResult* bohInterpResultSetStringViewStringViewPtr(bohInterpResult* pResult, const bohStringView* pStrView);
bohInterpResult* bohInterpResultSetNumber(bohInterpResult* pResult, bohNumber number);
bohInterpResult* bohInterpResultSetNumberPtr(bohInterpResult* pResult, const bohNumber* pNumber);
bohInterpResult* bohInterpResultSetNumberI64(bohInterpResult* pResult, int64_t value);
bohInterpResult* bohInterpResultSetNumberF64(bohInterpResult* pResult, double value);



typedef struct AST bohAST;

typedef struct Interpreter
{
    const bohAST* pAst;
} bohInterpreter;


bohInterpreter bohInterpCreate(const bohAST* pAst);
void bohInterpDestroy(bohInterpreter* pInterp);

bohNumber bohInterpInterpret(bohInterpreter* pInterp);
