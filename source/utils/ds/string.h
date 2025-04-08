#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef struct String
{
    char* pData;    
    size_t size;
    size_t capacity;
} bohString;


bohString bohStringCreate(void);
bohString bohStringCreateStr(const char* pCStr);
bohString bohStringCreateFromTo(const char* pBegin, const char* pEnd);

void bohStringDestroy(bohString* pStr);

void bohStringAssign(bohString* pDst, const bohString* pSrc);
void bohStringAssignStr(bohString* pDst, const char* pCStr);

const char* bohStringGetDataConst(const bohString* pStr);
char* bohStringGetData(bohString* pStr);

size_t bohStringGetSize(bohString* pStr);
size_t bohStringGetCapacity(bohString* pStr);

char bohStringAt(bohString* pStr, size_t index);

bool bohStringIsEmpty(bohString* pStr);
