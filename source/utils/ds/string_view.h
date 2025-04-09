#pragma once

#include <stdint.h>


typedef struct String bohString;


typedef struct StringView
{
    const char* pData;
    size_t size;
} bohStringView;


bohStringView bohStringViewCreate(void);
bohStringView bohStringViewCreateCStr(const char* pStr);
bohStringView bohStringViewCreateCStrSized(const char* pStr, size_t size);
bohStringView bohStringViewCreateString(const bohString* pStr);


void bohStringViewReset(bohStringView* pStringView);


bohStringView* bohStringViewAssign(bohStringView* pDst, const bohStringView* pSrc);
bohStringView* bohStringViewAssignCStr(bohStringView* pDst, const char* pStr);
bohStringView* bohStringViewAssignCStrSized(bohStringView* pDst, const char* pStr, size_t size);
bohStringView* bohStringViewAssignString(bohStringView* pDst, const bohString* pStr);


const char* bohStringViewGetData(const bohStringView* pStrView);
size_t bohStringViewGetSize(const bohStringView* pStrView);

char bohStringViewAt(const bohStringView* pStrView, size_t index);


bool bohStringViewIsEmpty(const bohStringView* pStrView);


bool bohStringViewEqual(const bohStringView* pLeft, const bohStringView* pRight);
bool bohStringViewLess(const bohStringView* pLeft, const bohStringView* pRight);
bool bohStringViewGreater(const bohStringView* pLeft, const bohStringView* pRight);
