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

    bohString str;

    str.pData = "";
    str.size = strlen(pCStr);
    str.capacity = str.size > 0 ? str.size + 1 : 0;

    if (str.capacity > 0) {
        str.pData = (char*)malloc(str.capacity);
        strcpy_s(str.pData, str.capacity * sizeof(char), pCStr);
    }

    return str;
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


void bohStringDestroy(bohString* pStr)
{
    assert(pStr);

    if (pStr->capacity > 0) {
        free(pStr->pData);
    }

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

    pDst->pData = (char*)malloc(pSrc->capacity);
    strcpy_s(pDst->pData, pSrc->capacity, pSrc->pData);

    pDst->size = pSrc->size;
    pDst->capacity = pSrc->capacity;

    return pDst;
}


bohString* bohStringAssignCStr(bohString* pDst, const char* pCStr)
{
    assert(pDst);

    if (!pCStr || strlen(pCStr) == 0) {
        *pDst = bohStringCreate();
        return pDst;
    }

    const size_t cStrLength = strlen(pCStr);
    const size_t cStrCapacity = cStrLength + 1;

    if (pDst->capacity < cStrCapacity) {
        bohStringDestroy(pDst);
        *pDst = bohStringCreateCStr(pCStr);
        return pDst;
    }

    memset(pDst->pData, 0, pDst->capacity);
    strcpy_s(pDst->pData, pDst->capacity, pCStr);
    pDst->size = cStrLength;

    return pDst;
}


const char* bohStringGetDataConst(const bohString* pStr)
{
    assert(pStr);
    return pStr->pData;
}


char* bohStringGetData(bohString *pStr)
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

    return bohStringViewEqual(&left, &right);
}


bool bohStringLess(const bohString* pLeft, const bohString* pRight)
{
    assert(pLeft);
    assert(pRight);

    bohStringView left = bohStringViewCreateString(pLeft);
    bohStringView right = bohStringViewCreateString(pRight);

    return bohStringViewLess(&left, &right);
}


bool bohStringGreater(const bohString* pLeft, const bohString* pRight)
{
    assert(pLeft);
    assert(pRight);

    bohStringView left = bohStringViewCreateString(pLeft);
    bohStringView right = bohStringViewCreateString(pRight);

    return bohStringViewGreater(&left, &right);
}