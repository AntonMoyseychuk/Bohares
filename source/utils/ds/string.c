#include "pch.h"

#include "string.h"


bohString bohStringCreate(void)
{
    bohString str;

    str.pData = "";
    str.size = 0;
    str.capacity = 0;

    return str;
}


bohString bohStringCreateStr(const char* pCStr)
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


void bohStringAssign(bohString* pDst, const bohString* pSrc)
{
    assert(pDst);
    assert(pSrc);

    bohStringDestroy(pDst);

    if (pSrc->capacity == 0) {
        *pDst = bohStringCreate();
        return;
    }

    pDst->pData = (char*)malloc(pSrc->capacity);
    strcpy_s(pDst->pData, pSrc->capacity, pSrc->pData);

    pDst->size = pSrc->size;
    pDst->capacity = pSrc->capacity;
}


void bohStringAssignStr(bohString* pDst, const char* pCStr)
{
    assert(pDst);
    assert(pCStr);

    const size_t cStrLength = strlen(pCStr);
    const size_t cStrCapacity = cStrLength + 1;

    if (pDst->capacity < cStrCapacity) {
        bohStringDestroy(pDst);
        *pDst = bohStringCreateStr(pCStr);
        return;
    }

    memset(pDst->pData, 0, pDst->capacity);
    strcpy_s(pDst->pData, pDst->capacity, pCStr);
    pDst->size = cStrLength;
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


size_t bohStringGetSize(bohString *pStr)
{
    assert(pStr);
    return pStr->size;
}


size_t bohStringGetCapacity(bohString *pStr)
{
    assert(pStr);
    return pStr->capacity;
}


char bohStringAt(bohString* pStr, size_t index)
{
    assert(pStr);
    assert(pStr->pData);
    assert(index < pStr->size);

    return pStr->pData[index];
}


bool bohStringIsEmpty(bohString* pStr)
{
    assert(pStr);
    return pStr->size == 0;
}
