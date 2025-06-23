#include "pch.h"

#include "state.h"

#include "core.h"


bohStackFrame bohStackFrameCreateChild(const bohStackFrame* pOwner)
{
    bohStackFrame frame = {0};

    frame.pOwner = pOwner;

    return frame;
}
