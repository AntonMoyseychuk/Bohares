#pragma once

#include "utils/ds/dyn_array.h"
#include "utils/ds/string_view.h"
#include "utils/ds/string.h"


typedef struct LexerError
{
    bohStringView filepath;
    bohString     message;
    uint64_t      line;
    uint64_t      column;
} bohLexerError;


bohLexerError bohLexerErrorCreate(void);
void bohLexerErrorDestroy(bohLexerError* pError);

bohLexerError* bohLexerErrorAssign(bohLexerError* pDst, const bohLexerError* pSrc);


typedef struct ParserError
{
    bohStringView filepath;
    bohString     message;
    uint64_t      line;
    uint64_t      column;
} bohParserError;


bohParserError bohParserErrorCreate(void);
void bohParserErrorDestroy(bohParserError* pError);

bohParserError* bohParserErrorAssign(bohParserError* pDst, const bohParserError* pSrc);


typedef struct State
{
    bohDynArray lexerErrors;
    bohDynArray parserErrors;

    bohStringView currProcessingFile;
} bohState;


void bohGlobalStateInit(void);
void bohGlobalStateDestroy(void);
bool bohGlobalStateIsInitialized(void);

bohState* bohGlobalStateGet(void);


void bohStateSetCurrProcessingFile(bohState* pState, bohStringView filepath);

void bohStateEmplaceLexerError(bohState* pState, uint64_t line, uint64_t column, const char* pMessage);
void bohStateEmplaceParserError(bohState* pState, uint64_t line, uint64_t column, const char* pMessage);

size_t bohStateGetLexerErrorsCount(const bohState* pState);
size_t bohStateGetParserErrorsCount(const bohState* pState);

bool bohStateHasLexerErrors(const bohState* pState);
bool bohStateHasParserErrors(const bohState* pState);

const bohLexerError* bohStateLexerErrorAt(const bohState* pState, size_t index);
const bohParserError* bohStateParserErrorAt(const bohState* pState, size_t index);
