#pragma once

#include "core.h"

#include "utils/ds/string_view.h"

#include <stdio.h>


typedef struct ErrorsState
{
    bohStringView currProcessingFile;

    bool lexerErrorsOccurred;
    bool parserErrorsOccurred;
    bool interpErrorsOccurred;
} bohErrorsState;


void bohErrorsStateInit(void);
void bohErrorsStateDestroy(void);
bool bohErrorsStateIsInitialized(void);

bohErrorsState* bohErrorsStateGet(void);

void bohErrorsStateSetCurrProcessingFile(bohErrorsState* pState, bohStringView filepath);
const bohStringView* bohErrorsStateGetCurrProcessingFile(const bohErrorsState* pState);

const bohStringView* bohErrorsStateGerCurrProcessingFileGlobal(void);

void bohErrorsStatePushLexerErrorGlobal(void);
void bohErrorsStatePushParserErrorGlobal(void);
void bohErrorsStatePushInterpreterErrorGlobal(void);


bool bohErrorsStateHasLexerErrorGlobal(void);
bool bohErrorsStateHasParserErrorGlobal(void);
bool bohErrorsStateHasInterpreterErrorGlobal(void);


void bohErrorsStatePrintError(
    FILE* pStream, 
    const bohStringView* pFilepath, 
    bohLineNmb line, 
    bohColumnNmb column,
    const char* pErrorPrefix,
    const char* pFmt,
    ...
);
