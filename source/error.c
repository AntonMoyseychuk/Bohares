#include "pch.h"

#include "core.h"
#include "error.h"


static bohErrorsState* pBohState = NULL;


void bohErrorsStateInit(void)
{
    if (bohErrorsStateIsInitialized()) {
        return;
    }

    pBohState = (bohErrorsState*)malloc(sizeof(bohErrorsState));
    BOH_ASSERT(pBohState);

    memset(pBohState, 0, sizeof(bohErrorsState));

    pBohState->currProcessingFile = bohStringViewCreate();

    pBohState->lexerErrorsOccurred = false;
    pBohState->parserErrorsOccurred = false;
    pBohState->interpErrorsOccurred = false;
}


void bohErrorsStateDestroy(void)
{
    if (!bohErrorsStateIsInitialized()) {
        return;
    }

    bohStringViewReset(&pBohState->currProcessingFile);

    pBohState->lexerErrorsOccurred = false;
    pBohState->parserErrorsOccurred = false;
    pBohState->interpErrorsOccurred = false;

    free(pBohState);
    pBohState = NULL;
}


bool bohErrorsStateIsInitialized(void)
{
    return pBohState != NULL;
}


bohErrorsState* bohErrorsStateGet(void)
{
    BOH_ASSERT(pBohState && "bohares errors state is not initialized");
    return pBohState;
}


void bohErrorsStateSetCurrProcessingFile(bohErrorsState* pState, bohStringView filepath)
{
    BOH_ASSERT(pState);
    bohStringViewAssignStringViewPtr(&pState->currProcessingFile, &filepath);
}


const bohStringView* bohErrorsStateGetCurrProcessingFile(const bohErrorsState* pState)
{
    BOH_ASSERT(pState);
    return &pState->currProcessingFile;
}


const bohStringView* bohErrorsStateGerCurrProcessingFileGlobal(void)
{
    return bohErrorsStateGetCurrProcessingFile(bohErrorsStateGet());
}


void bohErrorsStatePushLexerErrorGlobal(void)
{
    bohErrorsStateGet()->lexerErrorsOccurred = true;
}


void bohErrorsStatePushParserErrorGlobal(void)
{
    bohErrorsStateGet()->parserErrorsOccurred = true;
}


void bohErrorsStatePushInterpreterErrorGlobal(void)
{
    bohErrorsStateGet()->interpErrorsOccurred = true;
}


bool bohErrorsStateHasLexerErrorGlobal(void)
{
    return bohErrorsStateGet()->lexerErrorsOccurred;
}


bool bohErrorsStateHasParserErrorGlobal(void)
{
    return bohErrorsStateGet()->parserErrorsOccurred;
}


bool bohErrorsStateHasInterpreterErrorGlobal(void)
{
    return bohErrorsStateGet()->interpErrorsOccurred;
}


void bohErrorsStatePrintError(
    FILE* pStream, 
    const bohStringView* pFilepath, 
    bohLineNmb line, 
    bohColumnNmb column, 
    const char* pErrorPrefix, 
    const char* pFmt, 
    ...
) {
    BOH_ASSERT(pStream);
    BOH_ASSERT(pFilepath);
    BOH_ASSERT(pErrorPrefix);
    BOH_ASSERT(pFmt);

    fprintf_s(stderr, "%s[%s]:%s ", BOH_OUTPUT_COLOR_RED, pErrorPrefix, BOH_OUTPUT_COLOR_RESET);

    va_list args;
    va_start(args, pFmt);

    vfprintf_s(pStream, pFmt, args);
    va_end(args);

    fprintf_s(stderr, " (%.*s, %u:%u)\n", bohStringViewGetSize(pFilepath), bohStringViewGetData(pFilepath), line, column);
}
