#pragma once

#include <stdint.h>


typedef struct String bohString;


enum {
    BOH_STRING_VIEW_BITS_PER_SIZE = sizeof(size_t) * CHAR_BIT - 1
};


typedef struct StringView
{
    union {
        const char* pConstData;
        char* pData;
    };

    struct {
        size_t size : BOH_STRING_VIEW_BITS_PER_SIZE;
        size_t isConstantPtr : 1;
    };
} bohStringView;


size_t bohStringViewGetMaxSize(void);


bohStringView bohStringViewCreate(void);
bohStringView bohStringViewCreateCStr(char* pStr);
bohStringView bohStringViewCreateConstCStr(const char* pStr);
bohStringView bohStringViewCreateCStrSized(char* pStr, size_t size);
bohStringView bohStringViewCreateConstCStrSized(const char* pStr, size_t size);
bohStringView bohStringViewCreateString(bohString* pStr);
bohStringView bohStringViewCreateConstString(const bohString* pStr);
bohStringView bohStringViewCreateStringView(bohStringView strView);
bohStringView bohStringViewCreateStringViewPtr(const bohStringView* pStrView);

void bohStringViewReset(bohStringView* pStringView);

bohStringView* bohStringViewAssignStringView(bohStringView* pDst, bohStringView src);
bohStringView* bohStringViewAssignStringViewPtr(bohStringView* pDst, const bohStringView* pSrc);
bohStringView* bohStringViewAssignCStr(bohStringView* pDst, char* pStr);
bohStringView* bohStringViewAssignConstCStr(bohStringView* pDst, const char* pStr);
bohStringView* bohStringViewAssignCStrSized(bohStringView* pDst, char* pStr, size_t size);
bohStringView* bohStringViewAssignConstCStrSized(bohStringView* pDst, const char* pStr, size_t size);
bohStringView* bohStringViewAssignString(bohStringView* pDst, bohString* pStr);
bohStringView* bohStringViewAssignConstString(bohStringView* pDst, const bohString* pStr);

bohStringView* bohStringViewResize(bohStringView* pStrView, size_t newSize);

bohStringView* bohStringViewMove(bohStringView* pDst, bohStringView* pSrc);

const char* bohStringViewGetData(const bohStringView* pStrView);
size_t bohStringViewGetSize(const bohStringView* pStrView);

char bohStringViewAt(const bohStringView* pStrView, size_t index);
void bohStringViewSetAt(bohStringView* pStrView, char ch, size_t index);

bool bohStringViewIsEmpty(const bohStringView* pStrView);
bool bohStringViewIsConst(const bohStringView* pStrView);

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


void bohStringViewReplaceSymbols(bohStringView* pStrView, char targetSymb, char newSymb);
void bohStringViewRemoveSymbols(bohStringView* pStrView, char symb);
