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
bohString bohStringCreateStringView(bohStringView strView);
bohString bohStringCreateStringViewPtr(const bohStringView* pStrView);
bohString bohStringCreateString(const bohString* pString);

void bohStringDestroy(bohString* pStr);

bohString* bohStringAssign(bohString* pDst, const bohString* pSrc);
bohString* bohStringAssignSizedCStr(bohString* pDst, const char* pCStr, size_t length);
bohString* bohStringAssignCStr(bohString* pDst, const char* pCStr);
bohString* bohStringAssignStringView(bohString* pDst, bohStringView strView);
bohString* bohStringAssignStringViewPtr(bohString* pDst, const bohStringView* pStrView);

bohString* bohStringReserve(bohString* pStr, size_t newCapacity);
bohString* bohStringResize(bohString* pStr, size_t newSize);

bohString* bohStringMove(bohString* pDst, bohString* pSrc);

const char* bohStringGetCStr(const bohString* pStr);
size_t bohStringGetSize(const bohString* pStr);
size_t bohStringGetCapacity(const bohString* pStr);

char bohStringAt(const bohString* pStr, size_t index);
void bohStringSetAt(bohString* pStr, char ch, size_t index);

bool bohStringIsEmpty(const bohString* pStr);

bool bohStringEqual(const bohString* pLeft, const bohString* pRight);
bool bohStringNotEqual(const bohString* pLeft, const bohString* pRight);
bool bohStringLess(const bohString* pLeft, const bohString* pRight);
bool bohStringLessEqual(const bohString* pLeft, const bohString* pRight);
bool bohStringGreater(const bohString* pLeft, const bohString* pRight);
bool bohStringGreaterEqual(const bohString* pLeft, const bohString* pRight);

bohString bohStringAdd(const bohString* pLeft, const bohString* pRight);
bohString bohStringAddStringView(const bohString* pLeft, const bohStringView* pRStrView);
bohString bohStringViewAddString(const bohStringView* pLStrView, const bohString* pRight);
bohString bohStringViewAddStringView(const bohStringView* pLStrView, const bohStringView* pRStrView);
