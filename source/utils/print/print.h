#pragma once

#include <stdint.h>
#include <stdio.h>


typedef enum OutputColor
{
    BOH_OUTPUT_COLOR_BLACK,
    BOH_OUTPUT_COLOR_RED,
    BOH_OUTPUT_COLOR_GREEN,
    BOH_OUTPUT_COLOR_YELLOW,
    BOH_OUTPUT_COLOR_BLUE,
    BOH_OUTPUT_COLOR_MAGENTA,
    BOH_OUTPUT_COLOR_CYAN,
    BOH_OUTPUT_COLOR_WHITE,

    BOH_OUTPUT_COLOR_COUNT,
} bohOutputColor;


void bohColorPrintf(FILE* const pStream, bohOutputColor color, const char* pFmt, ...);

