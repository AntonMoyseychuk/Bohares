#pragma once

#include <stdio.h>


#define BOH_OUTPUT_COLOR_BLACK   "\033[30m"
#define BOH_OUTPUT_COLOR_RED     "\033[31m"
#define BOH_OUTPUT_COLOR_GREEN   "\033[32m"
#define BOH_OUTPUT_COLOR_YELLOW  "\033[33m"
#define BOH_OUTPUT_COLOR_BLUE    "\033[34m"
#define BOH_OUTPUT_COLOR_MAGENTA "\033[35m"
#define BOH_OUTPUT_COLOR_CYAN    "\033[36m"
#define BOH_OUTPUT_COLOR_WHITE   "\033[37m"
#define BOH_OUTPUT_COLOR_RESET   "\033[0m"


#if defined(_MSC_VER)
    #define BOH_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
    #define BOH_DEBUG_BREAK() __builtin_trap()
#else
    #error Currently, only MSVC and Clang are supported
#endif


#if defined(_DEBUG) || !defined(NDEBUG)
    #define BOH_DEBUG
#endif


#if defined(BOH_DEBUG)
    #define BOH_ASSERT_MSG(COND, MSG)                                                                                     \
        if (!(COND)) {                                                                                                    \
            fprintf_s(stderr, "%s%s: %s (%d)%s", BOH_OUTPUT_COLOR_RED, MSG, __FILE__, __LINE__, BOH_OUTPUT_COLOR_RESET);  \
            BOH_DEBUG_BREAK();                                                                                            \
        }

    #define BOH_ASSERT(COND)     BOH_ASSERT_MSG(COND, #COND)
    #define BOH_ASSERT_FAIL(MSG) BOH_ASSERT_MSG(false, MSG)
        
#else
    #define BOH_ASSERT_MSG(COND, MSG)
    #define BOH_ASSERT(COND)
    #define BOH_ASSERT_FAIL(MSG)
#endif


typedef uint32_t bohLineNmb;
typedef uint32_t bohColumnNmb;
