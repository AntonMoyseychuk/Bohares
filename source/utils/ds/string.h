#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef struct StringView bohStringView;


typedef struct String
{
    char* pData;    
    size_t size;
    size_t capacity;
} bohString;


bohString bohStringCreate(void);
bohString bohStringCreateCStr(const char* pCStr);
bohString bohStringCreateFromTo(const char* pBegin, const char* pEnd);

void bohStringDestroy(bohString* pStr);

bohString* bohStringAssign(bohString* pDst, const bohString* pSrc);
bohString* bohStringAssignCStr(bohString* pDst, const char* pCStr);

const char* bohStringGetDataConst(const bohString* pStr);
char* bohStringGetData(bohString* pStr);

size_t bohStringGetSize(bohString* pStr);
size_t bohStringGetCapacity(bohString* pStr);

char bohStringAt(bohString* pStr, size_t index);

bool bohStringIsEmpty(bohString* pStr);
