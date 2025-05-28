#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>

#include <memory.h>

#include <string.h>

#include <assert.h>

#include "utils/file/file.h"

#include "utils/ds/string.h"
#include "utils/ds/string_view.h"

#include "utils/ds/dyn_array.h"

#define BOH_OUTPUT_COLOR_BLACK   "\033[30m"
#define BOH_OUTPUT_COLOR_RED     "\033[31m"
#define BOH_OUTPUT_COLOR_GREEN   "\033[32m"
#define BOH_OUTPUT_COLOR_YELLOW  "\033[33m"
#define BOH_OUTPUT_COLOR_BLUE    "\033[34m"
#define BOH_OUTPUT_COLOR_MAGENTA "\033[35m"
#define BOH_OUTPUT_COLOR_CYAN    "\033[36m"
#define BOH_OUTPUT_COLOR_WHITE   "\033[37m"
#define BOH_OUTPUT_COLOR_RESET   "\033[0m"