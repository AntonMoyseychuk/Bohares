#pragma once

#include "types.h"


typedef enum InterpResultType
{
    BOH_INTERP_RES_TYPE_NUMBER,
    BOH_INTERP_RES_TYPE_STRING
} bohInterpResultType;


typedef struct InterpResult
{
    bohInterpResultType type;
    
    union {
        bohBoharesString string;
        bohNumber number;
    };
} bohInterpResult;


const char* bohInterpResultTypeToStr(const bohInterpResult* pResult);


bohInterpResult bohInterpResultCreate(void);

bohInterpResult bohInterpResultCreateNumberI64(int64_t value);
bohInterpResult bohInterpResultCreateNumberF64(double value);
bohInterpResult bohInterpResultCreateNumber(bohNumber number);
bohInterpResult bohInterpResultCreateNumberPtr(const bohNumber* pNumber);

bohInterpResult bohInterpResultCreateString(const bohString* pString);
bohInterpResult bohInterpResultCreateStringCStr(const char* pCStr);
bohInterpResult bohInterpResultCreateStringStringView(bohStringView strView);
bohInterpResult bohInterpResultCreateStringStringViewPtr(const bohStringView* pStrView);
bohInterpResult bohInterpResultCreateStringBoharesStringRVal(bohBoharesString string);
bohInterpResult bohInterpResultCreateStringBoharesStringRValPtr(bohBoharesString* pString);
bohInterpResult bohInterpResultCreateStringBoharesStringPtr(const bohBoharesString* pString);

bohInterpResult bohInterpResultCreateStringViewStringView(bohStringView strView);
bohInterpResult bohInterpResultCreateStringViewStringViewPtr(const bohStringView* pStrView);

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

bohInterpResult bohInterpInterpret(bohInterpreter* pInterp);
