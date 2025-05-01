#pragma once

#include <stdint.h>


void bohThrowCompileError(const char* pMsg, ...);


#define BOH_THROW_ERROR_FMT(FMT, ...) bohThrowCompileError(FMT,  __VA_ARGS__)
