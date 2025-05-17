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



typedef struct AST bohAST;

typedef struct Interpreter
{
    const bohAST* pAst;
} bohInterpreter;


bohInterpreter bohInterpCreate(const bohAST* pAst);
void bohInterpDestroy(bohInterpreter* pInterp);

bohNumber bohInterpInterpret(bohInterpreter* pInterp);
