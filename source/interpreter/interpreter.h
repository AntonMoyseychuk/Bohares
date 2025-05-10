#pragma once

#include "types.h"


typedef struct AST bohAST;


typedef struct Interpreter
{
    const bohAST* pAst;
} bohInterpreter;


bohInterpreter bohInterpCreate(const bohAST* pAst);
void bohInterpDestroy(bohInterpreter* pInterp);

bohNumber bohInterpInterpret(bohInterpreter* pInterp);
