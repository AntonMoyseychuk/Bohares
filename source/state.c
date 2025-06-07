#include "pch.h"

#include "core.h"
#include "state.h"


static bohState* pBohState = NULL;


static void lexErrorCopyFunc(void* pDst, const void* pSrc)
{
    bohLexerErrorAssign((bohLexerError*)pDst, (const bohLexerError*)pSrc);
}


static void lexErrorDefaultConstructor(void* pElement)
{
    *((bohLexerError*)pElement) = bohLexerErrorCreate();
}


static void lexErrorDestructor(void* pElement)
{
    bohLexerErrorDestroy((bohLexerError*)pElement);
}


static void parsErrorCopyFunc(void* pDst, const void* pSrc)
{
    bohParserErrorAssign((bohParserError*)pDst, (const bohParserError*)pSrc);
}


static void parsErrorDefaultConstructor(void* pElement)
{
    *((bohParserError*)pElement) = bohParserErrorCreate();
}


static void parsErrorDestructor(void* pElement)
{
    bohParserErrorDestroy((bohParserError*)pElement);
}


static void interpErrorCopyFunc(void* pDst, const void* pSrc)
{
    bohInterpreterErrorAssign((bohInterpreterError*)pDst, (const bohInterpreterError*)pSrc);
}


static void interpErrorDefaultConstructor(void* pElement)
{
    *((bohInterpreterError*)pElement) = bohInterpreterErrorCreate();
}


static void interpErrorDestructor(void* pElement)
{
    bohInterpreterErrorDestroy((bohInterpreterError*)pElement);
}


bohLexerError bohLexerErrorCreate(void)
{
    bohLexerError error = {0};
    error.filepath = bohStringViewCreate();
    error.message = bohStringCreate();
    error.line = 0;
    error.column = 0;

    return error;
}


void bohLexerErrorDestroy(bohLexerError* pError)
{
    BOH_ASSERT(pError);

    bohStringViewReset(&pError->filepath);
    bohStringDestroy(&pError->message);
    pError->line = 0;
    pError->column = 0;
}


bohLexerError* bohLexerErrorAssign(bohLexerError* pDst, const bohLexerError* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewAssignStringViewPtr(&pDst->filepath, &pSrc->filepath);
    bohStringAssign(&pDst->message, &pSrc->message);
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


bohParserError bohParserErrorCreate(void)
{
    bohParserError error = {0};
    error.filepath = bohStringViewCreate();
    error.message = bohStringCreate();
    error.line = 0;
    error.column = 0;

    return error;
}


void bohParserErrorDestroy(bohParserError* pError)
{
    BOH_ASSERT(pError);

    bohStringViewReset(&pError->filepath);
    bohStringDestroy(&pError->message);
    pError->line = 0;
    pError->column = 0;
}


bohParserError* bohParserErrorAssign(bohParserError* pDst, const bohParserError* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewAssignStringViewPtr(&pDst->filepath, &pSrc->filepath);
    bohStringAssign(&pDst->message, &pSrc->message);
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


bohInterpreterError bohInterpreterErrorCreate(void)
{
    bohInterpreterError error = {0};
    error.filepath = bohStringViewCreate();
    error.message = bohStringCreate();
    error.line = 0;
    error.column = 0;

    return error;
}


void bohInterpreterErrorDestroy(bohInterpreterError* pError)
{
    BOH_ASSERT(pError);

    bohStringViewReset(&pError->filepath);
    bohStringDestroy(&pError->message);
    pError->line = 0;
    pError->column = 0;
}


bohInterpreterError* bohInterpreterErrorAssign(bohInterpreterError* pDst, const bohInterpreterError* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewAssignStringViewPtr(&pDst->filepath, &pSrc->filepath);
    bohStringAssign(&pDst->message, &pSrc->message);
    pDst->line = pSrc->line;
    pDst->column = pSrc->column;

    return pDst;
}


void bohGlobalStateInit(void)
{
    if (bohGlobalStateIsInitialized()) {
        return;
    }

    pBohState = (bohState*)malloc(sizeof(bohState));
    BOH_ASSERT(pBohState);

    pBohState->lexerErrors = bohDynArrayCreate(sizeof(bohLexerError),
        lexErrorDefaultConstructor, lexErrorDestructor, lexErrorCopyFunc);

    pBohState->parserErrors = bohDynArrayCreate(sizeof(bohParserError),
        parsErrorDefaultConstructor, parsErrorDestructor, parsErrorCopyFunc);
        
    pBohState->interpreterErrors = bohDynArrayCreate(sizeof(bohInterpreterError),
        interpErrorDefaultConstructor, interpErrorDestructor, interpErrorCopyFunc);
}


void bohGlobalStateDestroy(void)
{
    if (!bohGlobalStateIsInitialized()) {
        return;
    }

    bohDynArrayDestroy(&pBohState->lexerErrors);
    bohDynArrayDestroy(&pBohState->parserErrors);

    free(pBohState);
    pBohState = NULL;
}


bool bohGlobalStateIsInitialized(void)
{
    return pBohState != NULL;
}


bohState* bohGlobalStateGet(void)
{
    return pBohState;
}


void bohStateSetCurrProcessingFile(bohState* pState, bohStringView filepath)
{
    BOH_ASSERT(pState);
    bohStringViewAssignStringViewPtr(&pState->currProcessingFile, &filepath);
}


void bohStateEmplaceLexerError(bohState* pState, uint64_t line, uint64_t column, const char* pMessage)
{
    BOH_ASSERT(pState);

    bohLexerError* pError = (bohLexerError*)bohDynArrayPushBackDummy(&pState->lexerErrors);
    
    bohStringViewAssignStringViewPtr(&pError->filepath, &pState->currProcessingFile);
    bohStringAssignCStr(&pError->message, pMessage);
    pError->line = line;
    pError->column = column;
}


void bohStateEmplaceParserError(bohState* pState, uint64_t line, uint64_t column, const char* pMessage)
{
    BOH_ASSERT(pState);

    bohParserError* pError = (bohParserError*)bohDynArrayPushBackDummy(&pState->parserErrors);
    
    bohStringViewAssignStringViewPtr(&pError->filepath, &pState->currProcessingFile);
    bohStringAssignCStr(&pError->message, pMessage);
    pError->line = line;
    pError->column = column;
}


void bohStateEmplaceInterpreterError(bohState *pState, uint64_t line, uint64_t column, const char *pMessage)
{
    BOH_ASSERT(pState);

    bohInterpreterError* pError = (bohInterpreterError*)bohDynArrayPushBackDummy(&pState->interpreterErrors);
    
    bohStringViewAssignStringViewPtr(&pError->filepath, &pState->currProcessingFile);
    bohStringAssignCStr(&pError->message, pMessage);
    pError->line = line;
    pError->column = column;
}


size_t bohStateGetLexerErrorsCount(const bohState* pState)
{
    BOH_ASSERT(pState);
    return bohDynArrayGetSize(&pState->lexerErrors);
}


size_t bohStateGetParserErrorsCount(const bohState* pState)
{
    BOH_ASSERT(pState);
    return bohDynArrayGetSize(&pState->parserErrors);
}


size_t bohStateGetInterpreterErrorsCount(const bohState* pState)
{
    BOH_ASSERT(pState);
    return bohDynArrayGetSize(&pState->interpreterErrors);
}


bool bohStateHasLexerErrors(const bohState* pState)
{
    return bohStateGetLexerErrorsCount(pState) > 0;
}


bool bohStateHasParserErrors(const bohState* pState)
{
    return bohStateGetParserErrorsCount(pState) > 0;
}


bool bohStateHasInterpreterErrors(const bohState* pState)
{
    return bohStateGetInterpreterErrorsCount(pState) > 0;
}


const bohLexerError* bohStateLexerErrorAt(const bohState* pState, size_t index)
{
    BOH_ASSERT(pState);
    return bohDynArrayAtConst(&pState->lexerErrors, index);
}


const bohParserError* bohStateParserErrorAt(const bohState* pState, size_t index)
{
    BOH_ASSERT(pState);
    return bohDynArrayAtConst(&pState->parserErrors, index);
}


const bohInterpreterError *bohStateInterpreterErrorAt(const bohState *pState, size_t index)
{
    BOH_ASSERT(pState);
    return bohDynArrayAtConst(&pState->interpreterErrors, index);
}
