#include "pch.h"

#include "interpreter.h"

#include "model.h"
#include "parser/parser.h"


static bohNumber interpInterpretAstNode(const bohAstNode* pNode)
{
    (void)pNode;

    return bohNumberCreateI64(0);
}


bohInterpreter bohInterpCreate(const bohAST* pAst)
{
    assert(pAst);

    bohInterpreter interp;
    interp.pAst = pAst;

    return interp;
}


void bohInterpDestroy(bohInterpreter* pInterp)
{
    assert(pInterp);
    pInterp->pAst = NULL;
}


void bohInterpInterpret(bohInterpreter* pInterp)
{
    assert(pInterp);

    const bohAST* pAst = pInterp->pAst;
    assert(pAst);

    if (bohAstIsEmpty(pAst)) {
        return;
    }

    interpInterpretAstNode(pAst->pRoot);
}
