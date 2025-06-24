#include "pch.h"

#include "state.h"

#include "core.h"


bohStackFrame bohStackFrameCreateChild(const bohStackFrame* pParentScope)
{
    bohStackFrame frame = {0};

    frame.pParentScope = pParentScope;

    return frame;
}
