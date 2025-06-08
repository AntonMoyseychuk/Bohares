#include "pch.h"

#include "core.h"

#include "string_view.h"
#include "string.h"


#define BOH_STRING_VIEW_MAX_SIZE (((size_t)1) << (BOH_STRING_VIEW_BITS_PER_SIZE))


size_t bohStringViewGetMaxSize(void)
{
    return BOH_STRING_VIEW_MAX_SIZE;
}


bohStringView bohStringViewCreate(void)
{
    bohStringView stringView;

    stringView.pConstData = NULL;
    stringView.isConstantPtr = true;
    stringView.size = 0;

    return stringView;
}


bohStringView bohStringViewCreateCStr(char *pStr)
{
    bohStringView stringView = bohStringViewCreate();
    bohStringViewAssignCStr(&stringView, pStr);

    return stringView;
}


bohStringView bohStringViewCreateConstCStr(const char* pStr)
{
    bohStringView stringView = bohStringViewCreate();
    bohStringViewAssignConstCStr(&stringView, pStr);

    return stringView;
}


bohStringView bohStringViewCreateCStrSized(char *pStr, size_t size)
{
    bohStringView stringView = bohStringViewCreate();
    bohStringViewAssignCStrSized(&stringView, pStr, size);

    return stringView;
}


bohStringView bohStringViewCreateConstCStrSized(const char* pStr, size_t size)
{
    bohStringView stringView = bohStringViewCreate();
    bohStringViewAssignConstCStrSized(&stringView, pStr, size);

    return stringView;
}


bohStringView bohStringViewCreateString(bohString* pStr)
{
    bohStringView stringView = bohStringViewCreate();
    bohStringViewAssignString(&stringView, pStr);

    return stringView;
}


bohStringView bohStringViewCreateConstString(const bohString* pStr)
{
    bohStringView stringView = bohStringViewCreate();
    bohStringViewAssignConstString(&stringView, pStr);

    return stringView;
}


bohStringView bohStringViewCreateStringView(bohStringView strView)
{
    return bohStringViewCreateStringViewPtr(&strView);
}


bohStringView bohStringViewCreateStringViewPtr(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);
    return *pStrView;
}


void bohStringViewReset(bohStringView* pStringView)
{
    BOH_ASSERT(pStringView);
    *pStringView = bohStringViewCreate();
}


bohStringView* bohStringViewAssignCStr(bohStringView* pDst, char* pStr)
{
    BOH_ASSERT(pDst);

    const size_t size = pStr ? strlen(pStr) : 0;
    BOH_ASSERT(size < BOH_STRING_VIEW_MAX_SIZE);

    pDst->pConstData = pStr;
    pDst->size = size;
    pDst->isConstantPtr = false;

    return pDst;
}


bohStringView* bohStringViewAssignConstCStr(bohStringView* pDst, const char* pStr)
{
    BOH_ASSERT(pDst);

    const size_t size = pStr ? strlen(pStr) : 0;
    BOH_ASSERT(size < BOH_STRING_VIEW_MAX_SIZE);

    pDst->pConstData = pStr;
    pDst->size = size;
    pDst->isConstantPtr = true;

    return pDst;
}


bohStringView* bohStringViewAssignCStrSized(bohStringView* pDst, char* pStr, size_t size)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(size < BOH_STRING_VIEW_MAX_SIZE);
    
    if (size > 0) {
        BOH_ASSERT(pStr != NULL);
    }

    pDst->pConstData = pStr;
    pDst->size = size;
    pDst->isConstantPtr = false;

    return pDst;
}


bohStringView* bohStringViewAssignConstCStrSized(bohStringView* pDst, const char* pStr, size_t size)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(size < BOH_STRING_VIEW_MAX_SIZE);

    pDst->pConstData = pStr;
    pDst->size = size;
    pDst->isConstantPtr = true;

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

    if (bohStringViewIsConst(pSrc)) {
        pDst->pConstData = pSrc->pConstData;
    } else {
        pDst->pData = pSrc->pData;
    }

    pDst->size = pSrc->size;
    pDst->isConstantPtr = pSrc->isConstantPtr;

    return pDst;
}


bohStringView* bohStringViewAssignString(bohStringView* pDst, bohString* pStr)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pStr);
    BOH_ASSERT(pStr->size < BOH_STRING_VIEW_MAX_SIZE);

    pDst->pConstData = pStr->pData;
    pDst->size = pStr->size;
    pDst->isConstantPtr = false;

    return pDst;
}


bohStringView* bohStringViewAssignConstString(bohStringView* pDst, const bohString* pStr)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pStr);
    BOH_ASSERT(pStr->size < BOH_STRING_VIEW_MAX_SIZE);

    pDst->pConstData = pStr->pData;
    pDst->size = pStr->size;
    pDst->isConstantPtr = true;

    return pDst;
}


bohStringView* bohStringViewResize(bohStringView* pStrView, size_t newSize)
{
    BOH_ASSERT(pStrView);
    BOH_ASSERT(newSize <= pStrView->size && "impossible to increase string view the size");

    pStrView->size = newSize;
    return pStrView;
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
    return pStrView->pData ? pStrView->pData : "";
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


void bohStringViewSetAt(bohStringView* pStrView, char ch, size_t index)
{
    BOH_ASSERT(pStrView);
    BOH_ASSERT(pStrView->pData);
    BOH_ASSERT(bohStringViewIsConst(pStrView));
    BOH_ASSERT(index < pStrView->size);

    pStrView->pData[index] = ch;
}


bool bohStringViewIsEmpty(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);
    return pStrView->size == 0;
}


bool bohStringViewIsConst(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);
    return pStrView->isConstantPtr;
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


void bohStringViewReplaceSymbols(bohStringView* pStrView, char targetSymb, char newSymb)
{
    BOH_ASSERT(pStrView);
    BOH_ASSERT(!bohStringViewIsConst(pStrView));
    
    const size_t strViewSize = pStrView->size;
    char* pStrViewData = pStrView->pData;

    for (size_t i = 0; i < strViewSize; ++i) {
        if (pStrViewData[i] == targetSymb) {
            pStrViewData[i] = newSymb;
        }
    }
}


void bohStringViewRemoveSymbols(bohStringView* pStrView, char symb)
{
    BOH_ASSERT(pStrView);
    BOH_ASSERT(!bohStringViewIsConst(pStrView));
    
    const size_t strViewSize = pStrView->size;
    char* pStrViewData = pStrView->pData;

    size_t i = 0;
    size_t j = 0;
    size_t newSize = strViewSize;

    while (i < strViewSize) {
        const char ch = pStrViewData[i];

        if (ch == symb) {
            newSize = newSize > 0 ? newSize - 1 : 0;
        } else {
            pStrViewData[j] = pStrViewData[i];
            ++j;
        }

        ++i;
    }

    memset(pStrViewData + newSize, 0, strViewSize - newSize);

    pStrView->size = newSize;
}
