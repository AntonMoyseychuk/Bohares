#include "pch.h"

#include "string.h"
#include "string_view.h"


bohString bohStringCreate(void)
{
    bohString str;

    str.pData = "";
    str.size = 0;
    str.capacity = 0;

    return str;
}


bohString bohStringCreateCStr(const char* pCStr)
{
    assert(pCStr);
    return bohStringCreateFromTo(pCStr, pCStr + strlen(pCStr));
}


bohString bohStringCreateFromTo(const char* pBegin, const char* pEnd)
{
    assert(pBegin);
    assert(pEnd);

    const uintptr_t beginAddr = (uintptr_t)pBegin;
    const uintptr_t endAddr = (uintptr_t)pEnd;

    assert(endAddr >= beginAddr);

    const size_t length = endAddr - beginAddr;

    bohString str;

    str.pData = "";
    str.size = length;
    str.capacity = str.size > 0 ? str.size + 1 : 0;

    if (str.capacity > 0) {
        str.pData = (char*)malloc(str.capacity);
        
        memcpy_s(str.pData, str.capacity, pBegin, length);
        str.pData[length] = '\0';
    }

    return str;
}


bohString bohStringCreateStringView(bohStringView strView)
{
    return bohStringCreateStringViewPtr(&strView);
}


bohString bohStringCreateStringViewPtr(const bohStringView* pStrView)
{
    assert(pStrView);

    const char* pStr = bohStringViewGetData(pStrView);
    const size_t size = bohStringViewGetSize(pStrView);

    return bohStringCreateFromTo(pStr, pStr + size);
}


bohString bohStringCreateString(const bohString* pString)
{
    assert(pString);

    bohString string;
    bohStringAssign(&string, pString);

    return string;
}


void bohStringDestroy(bohString* pStr)
{
    assert(pStr);

    if (pStr->capacity > 0) {
        free(pStr->pData);
    }

    pStr->pData = "";
    pStr->size = 0;
    pStr->capacity = 0;
}


bohString* bohStringAssign(bohString* pDst, const bohString* pSrc)
{
    assert(pDst);
    assert(pSrc);

    bohStringDestroy(pDst);

    if (pSrc->capacity == 0) {
        *pDst = bohStringCreate();
        return pDst;
    }

    pDst->size = pSrc->size;
    pDst->capacity = pSrc->capacity;

    pDst->pData = (char*)malloc(pDst->capacity);
    strcpy_s(pDst->pData, pDst->capacity, pSrc->pData);

    return pDst;
}


bohString* bohStringAssignSizedCStr(bohString* pDst, const char* pCStr, size_t length)
{
    assert(pDst);

    const size_t cStrCapacity = length + 1;

    if (pDst->capacity >= cStrCapacity) {
        memset(pDst->pData, 0, pDst->capacity);
        strcpy_s(pDst->pData, pDst->capacity, pCStr);
        pDst->size = length;

        return pDst;
    }

    bohStringDestroy(pDst);

    if (!pCStr || length == 0) {
        *pDst = bohStringCreate();
        return pDst;
    }

    bohStringDestroy(pDst);
    *pDst = bohStringCreateCStr(pCStr);
    return pDst;
}


bohString* bohStringAssignCStr(bohString* pDst, const char* pCStr)
{
    return bohStringAssignSizedCStr(pDst, pCStr, strlen(pCStr));
}


bohString* bohStringAssignStringView(bohString* pDst, bohStringView strView)
{
    return bohStringAssignStringViewPtr(pDst, &strView);
}


bohString* bohStringAssignStringViewPtr(bohString* pDst, const bohStringView* pStrView)
{
    return bohStringAssignSizedCStr(pDst, bohStringViewGetData(pStrView), bohStringViewGetSize(pStrView));
}


bohString* bohStringReserve(bohString* pStr, size_t newCapacity)
{
    assert(pStr);

    if (newCapacity <= pStr->capacity) {
        return pStr;
    }

    char* pNewBuff = (char*)malloc(newCapacity);
    strcpy_s(pNewBuff, newCapacity * sizeof(char), pStr->pData);

    free(pStr->pData);
    pStr->capacity = newCapacity;
    pStr->pData = pNewBuff;

    pNewBuff = NULL;

    return pStr;
}


bohString* bohStringResize(bohString* pStr, size_t newLength)
{
    assert(pStr);

    if (newLength == pStr->size) {
        return pStr;
    }

    const size_t oldCapacity = pStr->capacity;

    if (newLength >= oldCapacity) {
        const size_t newCapacity = newLength + 1;
        bohStringReserve(pStr, newCapacity);
    }

    pStr->size = newLength;

    return pStr;
}


bohString* bohStringMove(bohString* pDst, bohString* pSrc)
{
    assert(pDst);
    assert(pSrc);

    bohStringDestroy(pDst);

    pDst->pData = pSrc->pData;
    pDst->size = pSrc->size;
    pDst->capacity = pSrc->capacity;

    pSrc->pData = NULL;
    pSrc->size = 0;
    pSrc->capacity = 0;

    return pDst;
}


const char* bohStringGetCStr(const bohString* pStr)
{
    assert(pStr);
    return pStr->pData;
}


size_t bohStringGetSize(const bohString* pStr)
{
    assert(pStr);
    return pStr->size;
}


size_t bohStringGetCapacity(const bohString* pStr)
{
    assert(pStr);
    return pStr->capacity;
}


char bohStringAt(const bohString* pStr, size_t index)
{
    assert(pStr);
    assert(pStr->pData);
    assert(index < pStr->size);

    return pStr->pData[index];
}


bool bohStringIsEmpty(const bohString* pStr)
{
    assert(pStr);
    return pStr->size == 0;
}


bool bohStringEqual(const bohString* pLeft, const bohString* pRight)
{
    assert(pLeft);
    assert(pRight);

    bohStringView left = bohStringViewCreateString(pLeft);
    bohStringView right = bohStringViewCreateString(pRight);

    return bohStringViewEqualPtr(&left, &right);
}


bool bohStringNotEqual(const bohString* pLeft, const bohString* pRight)
{
    return !bohStringEqual(pLeft, pRight);
}


bool bohStringLess(const bohString* pLeft, const bohString* pRight)
{
    assert(pLeft);
    assert(pRight);

    bohStringView left = bohStringViewCreateString(pLeft);
    bohStringView right = bohStringViewCreateString(pRight);

    return bohStringViewLessPtr(&left, &right);
}


bool bohStringLessEqual(const bohString* pLeft, const bohString* pRight)
{
    assert(pLeft);
    assert(pRight);

    bohStringView left = bohStringViewCreateString(pLeft);
    bohStringView right = bohStringViewCreateString(pRight);

    return bohStringViewLessEqualPtr(&left, &right);
}


bool bohStringGreater(const bohString* pLeft, const bohString* pRight)
{
    assert(pLeft);
    assert(pRight);

    bohStringView left = bohStringViewCreateString(pLeft);
    bohStringView right = bohStringViewCreateString(pRight);

    return bohStringViewGreaterPtr(&left, &right);
}


bool bohStringGreaterEqual(const bohString* pLeft, const bohString* pRight)
{
    assert(pLeft);
    assert(pRight);

    bohStringView left = bohStringViewCreateString(pLeft);
    bohStringView right = bohStringViewCreateString(pRight);

    return bohStringViewGreaterEqualPtr(&left, &right);
}


bohString bohStringAdd(const bohString* pLeft, const bohString* pRight)
{
    const bohStringView leftStrView = bohStringViewCreateString(pLeft);
    const bohStringView rightStrView = bohStringViewCreateString(pRight);
    
    return bohStringViewAddStringView(&leftStrView, &rightStrView);
}


bohString bohStringAddStringView(const bohString* pLeft, const bohStringView* pRStrView)
{
    const bohStringView leftStrView = bohStringViewCreateString(pLeft);
    return bohStringViewAddStringView(&leftStrView, pRStrView);
}


bohString bohStringViewAddString(const bohStringView* pLStrView, const bohString* pRight)
{
    const bohStringView rightStrView = bohStringViewCreateString(pRight);
    return bohStringViewAddStringView(pLStrView, &rightStrView);
}


bohString bohStringViewAddStringView(const bohStringView* pLStrView, const bohStringView* pRStrView)
{
    assert(pLStrView);
    assert(pRStrView);

    const size_t leftStringSize = bohStringViewGetSize(pLStrView);
    const size_t rightStringSize = bohStringViewGetSize(pRStrView);
    const size_t newStringSize = leftStringSize + rightStringSize;
    const size_t newStringCapacity = newStringSize + 1;

    bohString newString = bohStringCreate();
    
    newString.pData = (char*)malloc(newStringSize);
    memset(newString.pData, 0, newStringSize);

    strncpy_s(newString.pData, newStringCapacity, pLStrView->pData, leftStringSize);
    strncpy_s(newString.pData + leftStringSize, newStringCapacity - leftStringSize, pRStrView->pData, rightStringSize);

    newString.size = newStringSize;
    newString.capacity = newStringCapacity;

    return newString;
}
