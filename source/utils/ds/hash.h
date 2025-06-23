#pragma once

#include <stdint.h>


typedef struct String bohString;
typedef struct StringView bohStringView;


uint64_t bohHashCStr(const char* pStr, size_t size);
uint64_t bohHashString(const bohString* pString);
uint64_t bohHashStringView(const bohStringView* pStrView);

uint64_t bohHashMem(const void* pData, size_t size);
