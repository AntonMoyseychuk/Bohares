#include "pch.h"

#include "core.h"

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
    BOH_ASSERT(pStrView);
    return *pStrView;
}


void bohStringViewReset(bohStringView* pStringView)
{
    BOH_ASSERT(pStringView);

    pStringView->pData = "";
    pStringView->size = 0;
}


bohStringView* bohStringViewAssignCStr(bohStringView* pDst, const char* pStr)
{
    BOH_ASSERT(pDst);

    pDst->pData = pStr ? pStr : "";
    pDst->size = pStr ? strlen(pStr) : 0;

    return pDst;
}


bohStringView* bohStringViewAssignCStrSized(bohStringView* pDst, const char* pStr, size_t size)
{
    BOH_ASSERT(pDst);

    if (!pStr) {
        BOH_ASSERT(size == 0 && "pStr is NULL but size is not 0");
    }

    pDst->pData = pStr ? pStr : "";
    pDst->size = size;

    return pDst;
}


bohStringView* bohStringViewAssignStringView(bohStringView* pDst, bohStringView src)
{
    return bohStringViewAssignStringViewPtr(pDst, &src);
}


bohStringView* bohStringViewAssignStringViewPtr(bohStringView* pDst, const bohStringView* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->pData = pSrc->pData;
    pDst->size = pSrc->size;

    return pDst;
}


bohStringView* bohStringViewAssignString(bohStringView* pDst, const bohString* pStr)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pStr);

    pDst->pData = pStr->pData;
    pDst->size = pStr->size;

    return pDst;
}


bohStringView* bohStringViewMove(bohStringView* pDst, bohStringView* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringViewAssignStringViewPtr(pDst, pSrc);
    bohStringViewReset(pSrc);

    return pDst;
}


const char* bohStringViewGetData(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);
    return pStrView->pData;
}


size_t bohStringViewGetSize(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);
    return pStrView->size;
}


char bohStringViewAt(const bohStringView* pStrView, size_t index)
{
    BOH_ASSERT(pStrView);
    BOH_ASSERT(pStrView->pData);
    BOH_ASSERT(index < pStrView->size);

    return pStrView->pData[index];
}


bool bohStringViewIsEmpty(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);
    return pStrView->size == 0;
}


int32_t bohStringViewCmp(const bohStringView* pLeft, bohStringView right)
{
    return bohStringViewCmpPtr(pLeft, &right);
}


int32_t bohStringViewCmpPtr(const bohStringView *pLeft, const bohStringView *pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    const size_t minSize = (pLeft->size < pRight->size) ? pLeft->size : pRight->size;

    const int32_t cmpResult = strncmp(pLeft->pData, pRight->pData, minSize);
    
    if (cmpResult != 0) {
        return cmpResult;
    }
    
    if (pLeft->size == pRight->size) {
        return 0;
    }

    return pLeft->size < pRight->size ? -1 : 1;
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
