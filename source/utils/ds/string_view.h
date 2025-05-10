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
bohStringView bohStringViewCreateStringView(bohStringView strView);
bohStringView bohStringViewCreateStringViewPtr(const bohStringView* pStrView);

void bohStringViewReset(bohStringView* pStringView);

bohStringView* bohStringViewAssign(bohStringView* pDst, bohStringView src);
bohStringView* bohStringViewAssignPtr(bohStringView* pDst, const bohStringView* pSrc);
bohStringView* bohStringViewAssignCStr(bohStringView* pDst, const char* pStr);
bohStringView* bohStringViewAssignCStrSized(bohStringView* pDst, const char* pStr, size_t size);
bohStringView* bohStringViewAssignString(bohStringView* pDst, const bohString* pStr);

const char* bohStringViewGetData(const bohStringView* pStrView);
size_t bohStringViewGetSize(const bohStringView* pStrView);

char bohStringViewAt(const bohStringView* pStrView, size_t index);

bool bohStringViewIsEmpty(const bohStringView* pStrView);

int32_t bohStringViewCmp(const bohStringView* pLeft, bohStringView right);
int32_t bohStringViewCmpPtr(const bohStringView* pLeft, const bohStringView* pRight);

bool bohStringViewEqual(const bohStringView* pLeft, bohStringView right);
bool bohStringViewEqualPtr(const bohStringView* pLeft, const bohStringView* pRight);
bool bohStringViewNotEqual(const bohStringView* pLeft, bohStringView right);
bool bohStringViewNotEqualPtr(const bohStringView* pLeft, const bohStringView* pRight);
bool bohStringViewLess(const bohStringView* pLeft, bohStringView right);
bool bohStringViewLessPtr(const bohStringView* pLeft, const bohStringView* pRight);
bool bohStringViewLessEqual(const bohStringView* pLeft, bohStringView right);
bool bohStringViewLessEqualPtr(const bohStringView* pLeft, const bohStringView* pRight);
bool bohStringViewGreater(const bohStringView* pLeft, bohStringView right);
bool bohStringViewGreaterPtr(const bohStringView* pLeft, const bohStringView* pRight);
bool bohStringViewGreaterEqual(const bohStringView* pLeft, bohStringView right);
bool bohStringViewGreaterEqualPtr(const bohStringView* pLeft, const bohStringView* pRight);
