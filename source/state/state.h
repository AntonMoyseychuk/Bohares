#pragma once

#include "utils/ds/strid.h"


typedef enum SymbolType
{
    BOH_SYMBOL_TYPE_VAR,
    BOH_SYMBOL_TYPE_FUNC,
} bohSymbolType;


typedef struct Symbol
{
    bohStrID      name;
    size_t        varSize;
    bohSymbolType type;
} bohSymbol;


typedef struct StackFrame
{
    struct StackFrame* pParentScope;
} bohStackFrame;


bohStackFrame bohStackFrameCreateChild(const bohStackFrame* pParentScope);