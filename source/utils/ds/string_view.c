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


bohStringView *bohStringViewAssignStringView(bohStringView *pDst, const bohStringView *pSrc)
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


bool bohStringViewEqual(const bohStringView* pLeft, const bohStringView* pRight)
{
    assert(pLeft);
    assert(pRight);

    const size_t minSize = min(pLeft->size, pRight->size);

    const int cmpResult = _memicmp(pLeft->pData, pRight->pData, minSize);

    return cmpResult == 0 ? pLeft->size == pRight->size : false;
}


bool bohStringViewLess(const bohStringView *pLeft, const bohStringView *pRight)
{
    assert(pLeft);
    assert(pRight);

    const size_t minSize = min(pLeft->size, pRight->size);

    const int cmpResult = _memicmp(pLeft->pData, pRight->pData, minSize);

    if (cmpResult < 0) {
        return true;
    }

    return cmpResult == 0 ? pLeft->size < pRight->size : false;
}


bool bohStringViewGreater(const bohStringView *pLeft, const bohStringView *pRight)
{
    return !bohStringViewEqual(pLeft, pRight) && !bohStringViewLess(pLeft, pRight);
}
