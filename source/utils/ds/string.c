#include "pch.h"

#include "core.h"

#include "string.h"
#include "string_view.h"


bohString bohStringCreate(void)
{
    bohString str;

    str.pData = NULL;
    str.size = 0;
    str.capacity = 0;

    return str;
}


bohString bohStringCreateCStr(const char* pCStr)
{
    BOH_ASSERT(pCStr);
    return bohStringCreateFromTo(pCStr, pCStr + strlen(pCStr));
}


bohString bohStringCreateFromTo(const char* pBegin, const char* pEnd)
{
    BOH_ASSERT(pBegin);
    BOH_ASSERT(pEnd);

    const uintptr_t beginAddr = (uintptr_t)pBegin;
    const uintptr_t endAddr = (uintptr_t)pEnd;

    BOH_ASSERT(endAddr >= beginAddr);

    const size_t length = endAddr - beginAddr;

    bohString str;

    str.pData = NULL;
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
    BOH_ASSERT(pStrView);

    const char* pStr = bohStringViewGetData(pStrView);
    const size_t size = bohStringViewGetSize(pStrView);

    return bohStringCreateFromTo(pStr, pStr + size);
}


bohString bohStringCreateString(const bohString* pString)
{
    BOH_ASSERT(pString);

    bohString string;
    bohStringAssign(&string, pString);

    return string;
}


void bohStringDestroy(bohString* pStr)
{
    BOH_ASSERT(pStr);

    free(pStr->pData);

    pStr->pData = NULL;
    pStr->size = 0;
    pStr->capacity = 0;
}


bohString* bohStringAssign(bohString* pDst, const bohString* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohStringDestroy(pDst);

    if (pSrc->capacity == 0) {
        *pDst = bohStringCreate();
        return pDst;
    }

    pDst->size = pSrc->size;
    pDst->capacity = pSrc->capacity;

    pDst->pData = (char*)malloc(pDst->capacity);
    memcpy_s(pDst->pData, pDst->capacity, pSrc->pData, pSrc->capacity);

    return pDst;
}


bohString* bohStringAssignSizedCStr(bohString* pDst, const char* pCStr, size_t length)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pCStr);

    const size_t cStrCapacity = length + 1;

    if (pDst->capacity >= cStrCapacity) {
        memcpy_s(pDst->pData, pDst->capacity, pCStr, length);
        pDst->pData[length] = '\0';
        pDst->size = length;

        return pDst;
    }

    bohStringDestroy(pDst);

    if (length == 0) {
        *pDst = bohStringCreate();
        return pDst;
    }

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
    BOH_ASSERT(pStr);

    const size_t strSize = pStr->size;
    const size_t strCapacity = pStr->capacity;

    if (newCapacity <= strCapacity) {
        return pStr;
    }

    char* pNewBuff = (char*)malloc(newCapacity);
    BOH_ASSERT(pNewBuff);

    memcpy_s(pNewBuff, newCapacity, pStr->pData, strSize);
    pNewBuff[strSize] = '\0';

    bohStringDestroy(pStr);

    pStr->pData = pNewBuff;
    pStr->size = strSize;
    pStr->capacity = newCapacity;

    pNewBuff = NULL;

    return pStr;
}


bohString* bohStringResize(bohString* pStr, size_t newLength)
{
    BOH_ASSERT(pStr);

    if (newLength == pStr->size) {
        return pStr;
    }

    const size_t oldCapacity = pStr->capacity;

    if (newLength >= oldCapacity) {
        const size_t newCapacity = newLength + 1;
        bohStringReserve(pStr, newCapacity);
    }

    pStr->size = newLength;
    pStr->pData[newLength] = '\0';

    return pStr;
}


bohString* bohStringMove(bohString* pDst, bohString* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

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
    BOH_ASSERT(pStr);
    return pStr->pData ? pStr->pData : "";
}


size_t bohStringGetSize(const bohString* pStr)
{
    BOH_ASSERT(pStr);
    return pStr->size;
}


size_t bohStringGetCapacity(const bohString* pStr)
{
    BOH_ASSERT(pStr);
    return pStr->capacity;
}


char bohStringAt(const bohString* pStr, size_t index)
{
    BOH_ASSERT(pStr);
    BOH_ASSERT(pStr->pData);
    BOH_ASSERT(index < pStr->size);

    return pStr->pData[index];
}


void bohStringSetAt(bohString *pStr, char ch, size_t index)
{
    BOH_ASSERT(pStr);
    BOH_ASSERT(pStr->pData);
    BOH_ASSERT(index < pStr->size);

    pStr->pData[index] = ch;
}


bool bohStringIsEmpty(const bohString* pStr)
{
    BOH_ASSERT(pStr);
    return pStr->size == 0;
}


bool bohStringEqual(const bohString* pLeft, const bohString* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    bohStringView left = bohStringViewCreateConstString(pLeft);
    bohStringView right = bohStringViewCreateConstString(pRight);

    return bohStringViewEqualPtr(&left, &right);
}


bool bohStringNotEqual(const bohString* pLeft, const bohString* pRight)
{
    return !bohStringEqual(pLeft, pRight);
}


bool bohStringLess(const bohString* pLeft, const bohString* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    bohStringView left = bohStringViewCreateConstString(pLeft);
    bohStringView right = bohStringViewCreateConstString(pRight);

    return bohStringViewLessPtr(&left, &right);
}


bool bohStringLessEqual(const bohString* pLeft, const bohString* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    bohStringView left = bohStringViewCreateConstString(pLeft);
    bohStringView right = bohStringViewCreateConstString(pRight);

    return bohStringViewLessEqualPtr(&left, &right);
}


bool bohStringGreater(const bohString* pLeft, const bohString* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    bohStringView left = bohStringViewCreateConstString(pLeft);
    bohStringView right = bohStringViewCreateConstString(pRight);

    return bohStringViewGreaterPtr(&left, &right);
}


bool bohStringGreaterEqual(const bohString* pLeft, const bohString* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    bohStringView left = bohStringViewCreateConstString(pLeft);
    bohStringView right = bohStringViewCreateConstString(pRight);

    return bohStringViewGreaterEqualPtr(&left, &right);
}


bohString bohStringAdd(const bohString* pLeft, const bohString* pRight)
{
    const bohStringView leftStrView = bohStringViewCreateConstString(pLeft);
    const bohStringView rightStrView = bohStringViewCreateConstString(pRight);
    
    return bohStringViewAddStringView(&leftStrView, &rightStrView);
}


bohString bohStringAddStringView(const bohString* pLeft, const bohStringView* pRStrView)
{
    const bohStringView leftStrView = bohStringViewCreateConstString(pLeft);
    return bohStringViewAddStringView(&leftStrView, pRStrView);
}


bohString bohStringViewAddString(const bohStringView* pLStrView, const bohString* pRight)
{
    const bohStringView rightStrView = bohStringViewCreateConstString(pRight);
    return bohStringViewAddStringView(pLStrView, &rightStrView);
}


bohString bohStringViewAddStringView(const bohStringView* pLStrView, const bohStringView* pRStrView)
{
    BOH_ASSERT(pLStrView);
    BOH_ASSERT(pRStrView);

    const size_t leftStringSize = bohStringViewGetSize(pLStrView);
    const size_t rightStringSize = bohStringViewGetSize(pRStrView);
    const size_t newStringSize = leftStringSize + rightStringSize;
    const size_t newStringCapacity = newStringSize + 1;

    bohString newString = bohStringCreate();
    
    newString.pData = (char*)malloc(newStringCapacity);
    memset(newString.pData, 0, newStringCapacity);

    memcpy_s(newString.pData, newStringCapacity, bohStringViewGetData(pLStrView), leftStringSize);
    memcpy_s(newString.pData + leftStringSize, newStringCapacity - leftStringSize, bohStringGetCStr(pRStrView), rightStringSize);
    newString.pData[newStringSize] = '\0';

    newString.size = newStringSize;
    newString.capacity = newStringCapacity;

    return newString;
}
