#pragma once


typedef struct StackFrame
{
    struct StackFrame* pOwner;
} bohStackFrame;


bohStackFrame bohStackFrameCreateChild(const bohStackFrame* pOwner);