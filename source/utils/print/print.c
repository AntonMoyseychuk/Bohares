#include "pch.h"

#include "print.h"


static const char* BOH_OUTPUT_COLOR_STRS[] = {
    "\033[30m",
    "\033[31m",
    "\033[32m",
    "\033[33m",
    "\033[34m",
    "\033[35m",
    "\033[36m",
    "\033[37m"
};

static const char* BOH_OUTPUT_COLOR_RESET_STR = "\033[0m";


enum {
    BOH_MAX_FMT_LENGTH = 2047
};


static const char* bohOutputColorToStr(bohOutputColor color)
{
    assert(color < BOH_OUTPUT_COLOR_COUNT && "Invalid output color code");
    return BOH_OUTPUT_COLOR_STRS[color];
}


void bohColorPrintf(FILE* const pStream, bohOutputColor color, const char* pFmt, ...)
{
    assert(pStream);
    assert(pFmt);

    assert(strlen(pFmt) <= BOH_MAX_FMT_LENGTH);

    char fmt[BOH_MAX_FMT_LENGTH + 1] = {0};
    sprintf_s(fmt, BOH_MAX_FMT_LENGTH, "%s%s%s", bohOutputColorToStr(color), pFmt, BOH_OUTPUT_COLOR_RESET_STR);

    va_list args;
    va_start(args, pFmt);

    vfprintf_s(pStream, fmt, args);
    fflush(pStream);
    
    va_end(args);
}
