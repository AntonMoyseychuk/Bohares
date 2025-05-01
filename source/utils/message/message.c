#include "pch.h"

#include "message.h"


#define BOH_OUTPUT_COLOR_RESET_ASCII_CODE   "\033[0m"
#define BOH_OUTPUT_COLOR_BLACK_ASCII_CODE   "\033[30m"
#define BOH_OUTPUT_COLOR_RED_ASCII_CODE     "\033[31m"
#define BOH_OUTPUT_COLOR_GREEN_ASCII_CODE   "\033[32m"
#define BOH_OUTPUT_COLOR_YELLOW_ASCII_CODE  "\033[33m"
#define BOH_OUTPUT_COLOR_BLUE_ASCII_CODE    "\033[34m"
#define BOH_OUTPUT_COLOR_MAGENTA_ASCII_CODE "\033[35m"
#define BOH_OUTPUT_COLOR_CYAN_ASCII_CODE    "\033[36m"
#define BOH_OUTPUT_COLOR_WHITE_ASCII_CODE   "\033[37m"


void bohThrowCompileError(const char* pMsg, ...)
{
    char message[2048] = { 0 };
    sprintf_s(message, sizeof(message), "%s%s%s", BOH_OUTPUT_COLOR_RED_ASCII_CODE, pMsg, BOH_OUTPUT_COLOR_RESET_ASCII_CODE);

    va_list args;
    va_start(args, pMsg);
    
    vfprintf_s(stderr, message, args);
    fflush(stderr);
    
    va_end(args);

    exit(-1);
}
