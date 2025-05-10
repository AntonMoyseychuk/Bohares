#include "pch.h"

#include "string_view.h"
#include "string.h"


bohStringView bohStringViewCreate(void)
{
    bohStringView stringView;

    stringView.pData = "";
    stringView.size = 0;

    return stringView;
}


bohStringView bohStringViewCreateCStr(const char* pStr)
{
    bohStringView stringView;

    bohStringViewAssignCStr(&stringView, pStr);

    return stringView;
}


bohStringView bohStringViewCreateCStrSized(const char* pStr, size_t size)
{
    bohStringView stringView;

    bohStringViewAssignCStrSized(&stringView, pStr, size);

    return stringView;
}


bohStringView bohStringViewCreateString(const bohString* pStr)
{
    bohStringView stringView;

    bohStringViewAssignString(&stringView, pStr);

    return stringView;
}


bohStringView bohStringViewCreateStringView(bohStringView strView)
{
    return strView;
}


bohStringView bohStringViewCreateStringViewPtr(const bohStringView* pStrView)
{
    assert(pStrView);
    return *pStrView;
}


void bohStringViewReset(bohStringView* pStringView)
{
    assert(pStringView);

    pStringView->pData = "";
    pStringView->size = 0;
}


bohStringView* bohStringViewAssignCStr(bohStringView* pDst, const char* pStr)
{
    assert(pDst);

    pDst->pData = pStr ? pStr : "";
    pDst->size = pStr ? strlen(pStr) : 0;

    return pDst;
}


bohStringView* bohStringViewAssignCStrSized(bohStringView* pDst, const char* pStr, size_t size)
{
    assert(pDst);

    if (!pStr) {
        assert(size == 0 && "pStr is NULL but size is not 0");
    }

    pDst->pData = pStr ? pStr : "";
    pDst->size = size;

    return pDst;
}


bohStringView* bohStringViewAssign(bohStringView* pDst, bohStringView src)
{
    return bohStringViewAssignPtr(pDst, &src);
}


bohStringView* bohStringViewAssignPtr(bohStringView* pDst, const bohStringView* pSrc)
{
    assert(pDst);
    assert(pSrc);

    pDst->pData = pSrc->pData;
    pDst->size = pSrc->size;

    return pDst;
}


bohStringView* bohStringViewAssignString(bohStringView* pDst, const bohString* pStr)
{
    assert(pDst);
    assert(pStr);

    pDst->pData = pStr->pData;
    pDst->size = pStr->size;

    return pDst;
}


const char* bohStringViewGetData(const bohStringView* pStrView)
{
    assert(pStrView);
    return pStrView->pData;
}


size_t bohStringViewGetSize(const bohStringView* pStrView)
{
    assert(pStrView);
    return pStrView->size;
}


char bohStringViewAt(const bohStringView* pStrView, size_t index)
{
    assert(pStrView);
    assert(pStrView->pData);
    assert(index < pStrView->size);

    return pStrView->pData[index];
}


bool bohStringViewIsEmpty(const bohStringView* pStrView)
{
    assert(pStrView);
    return pStrView->size == 0;
}


int32_t bohStringViewCmp(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewCmpPtr(pLeft, &right);
}


int32_t bohStringViewCmpPtr(const bohStringView *pLeft, const bohStringView *pRight)
{
    assert(pLeft);
    assert(pRight);

    const size_t minSize = (pLeft->size < pRight->size) ? pLeft->size : pRight->size;

    return strncmp(pLeft->pData, pRight->pData, minSize);
}


bool bohStringViewEqual(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewEqualPtr(pLeft, &right);
}


bool bohStringViewEqualPtr(const bohStringView *pLeft, const bohStringView *pRight)
{
    return bohStringViewCmpPtr(pLeft, pRight) == 0 ? pLeft->size == pRight->size : false;
}


bool bohStringViewNotEqual(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewNotEqualPtr(pLeft, &right);
}


bool bohStringViewNotEqualPtr(const bohStringView* pLeft, const bohStringView* pRight)
{
    return !bohStringViewEqualPtr(pLeft, pRight);
}


bool bohStringViewLess(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewLessPtr(pLeft, &right);
}


bool bohStringViewLessPtr(const bohStringView *pLeft, const bohStringView *pRight)
{
    const int32_t cmpResult = bohStringViewCmpPtr(pLeft, pRight);

    if (cmpResult == 0) {
        return pLeft->size < pRight->size;
    }

    return cmpResult < 0;
}


bool bohStringViewLessEqual(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewLessEqualPtr(pLeft, &right);
}


bool bohStringViewLessEqualPtr(const bohStringView* pLeft, const bohStringView* pRight)
{
    const int32_t cmpResult = bohStringViewCmpPtr(pLeft, pRight);

    if (cmpResult == 0) {
        return pLeft->size <= pRight->size;
    }

    return cmpResult < 0;
}


bool bohStringViewGreater(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewGreaterPtr(pLeft, &right);
}


bool bohStringViewGreaterPtr(const bohStringView* pLeft, const bohStringView* pRight)
{
    const int32_t cmpResult = bohStringViewCmpPtr(pLeft, pRight);

    if (cmpResult == 0) {
        return pLeft->size > pRight->size;
    }

    return cmpResult > 0;
}


bool bohStringViewGreaterEqual(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewGreaterEqualPtr(pLeft, &right);
}


bool bohStringViewGreaterEqualPtr(const bohStringView* pLeft, const bohStringView* pRight)
{
    const int32_t cmpResult = bohStringViewCmpPtr(pLeft, pRight);

    if (cmpResult == 0) {
        return pLeft->size >= pRight->size;
    }

    return cmpResult > 0;
}
