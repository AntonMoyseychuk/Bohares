#include "pch.h"

#include "core.h"

#include "dyn_array.h"


#define BOH_GET_DYN_ARRAY_ELEMENT_PTR(DATA, INDEX, ELEM_SIZE) (void*)((uint8_t*)(DATA) + (INDEX) * (ELEM_SIZE))


bohDynArray bohDynArrayCreate(size_t elementSize, 
    const bohDynArrElemDefConstr pConstr, 
    const bohDynArrElemDestr pDestr, 
    const bohDynArrElemCopyFunc pCopyFunc)
{
    BOH_ASSERT(elementSize > 0);

    if (!pConstr || !pDestr || !pCopyFunc) {
        BOH_ASSERT_MSG(pConstr == NULL && pDestr == NULL && pCopyFunc == NULL, 
            "If any of inner funtions is NULL than it means that element type is trivial and all functions must be NULL");
    }

    bohDynArray array;
    array.pData = NULL;
    array.size = 0;
    array.capacity = 0;

    array.elementSize = elementSize;
    array.pElemDefContr = pConstr;
    array.pElemDestr = pDestr;
    array.pElemCopyFunc = pCopyFunc;

    return array;
}


void bohDynArrayDestroy(bohDynArray* pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));

    bohDynArrayResize(pArray, 0);

    free(pArray->pData);
    pArray->pData = NULL;
    pArray->capacity = 0;

    pArray->elementSize = 0;
    pArray->pElemDefContr = NULL;
    pArray->pElemDestr = NULL;
    pArray->pElemCopyFunc = NULL;
}


void bohDynArrayReserve(bohDynArray* pArray, size_t newCapacity)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));

    if (newCapacity <= pArray->capacity) {
        return;
    }

    const size_t oldCapacityInBytes = pArray->capacity * pArray->elementSize;
    const size_t newCapacityInBytes = newCapacity * pArray->elementSize;
    const size_t elemSize = pArray->elementSize;

    void* pNewBuffer = malloc(newCapacityInBytes);
    BOH_ASSERT(pNewBuffer);

    memset(pNewBuffer, 0, newCapacityInBytes);

    const size_t arrSize = pArray->size;
    void* pOldBuffer = pArray->pData;

    const bohDynArrElemCopyFunc ElemCopyFunc = pArray->pElemCopyFunc;
    
    if (ElemCopyFunc) {
        
        for (size_t i = 0; i < arrSize; ++i) {
            void* pDst = BOH_GET_DYN_ARRAY_ELEMENT_PTR(pNewBuffer, i, elemSize);
            const void* pSrc = BOH_GET_DYN_ARRAY_ELEMENT_PTR(pOldBuffer, i, elemSize);
    
            ElemCopyFunc(pDst, pSrc);
        }
    } else {
        memcpy_s(pNewBuffer, newCapacityInBytes, pOldBuffer, oldCapacityInBytes);
    }
    
    pArray->pData = pNewBuffer;
    pArray->capacity = newCapacity;

    const bohDynArrElemDestr ElemDestr = pArray->pElemDestr;

    if (ElemDestr) {
        for (size_t i = 0; i < arrSize; ++i) {
            ElemDestr(BOH_GET_DYN_ARRAY_ELEMENT_PTR(pOldBuffer, i, elemSize));
        }
    }

    free(pOldBuffer);
}


void bohDynArrayResize(bohDynArray* pArray, size_t newSize)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));

    const size_t oldSize = pArray->size;

    if (newSize == oldSize) {
        return;
    }

    void* pArrData = pArray->pData;
    const size_t elemSize = pArray->elementSize;

    if (newSize < oldSize) {
        const bohDynArrElemDestr ElemDestructor = pArray->pElemDestr;

        if (ElemDestructor) {
            for (size_t i = newSize; i < oldSize; ++i) {
                ElemDestructor(BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArrData, i, elemSize));
            }
        }

        pArray->size = newSize;

        return;
    }

    bohDynArrayReserve(pArray, newSize);
    pArrData = pArray->pData;

    const bohDynArrElemDefConstr ElemConstructor = pArray->pElemDefContr;

    if (ElemConstructor) {
        for (size_t i = oldSize; i < newSize; ++i) {
            ElemConstructor(BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArrData, i, elemSize));
        }
    }

    pArray->size = newSize;
}


void bohDynArrayClear(bohDynArray* pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    bohDynArrayResize(pArray, 0);
}


void* bohDynArrayPushBackDummy(bohDynArray *pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));

    const size_t currSize = pArray->size; 
    const size_t currCapacity = pArray->capacity; 

    if (currSize + 1 > currCapacity) {
        const bool isZeroSized = currSize == 0;

        const size_t newSize = (currSize + (size_t)isZeroSized) * 2;
        bohDynArrayReserve(pArray, newSize);
    }

    ++pArray->size;

    return bohDynArrayAt(pArray, pArray->size - 1);
}


void* bohDynArrayPushBack(bohDynArray* pArray, const void *pData)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    BOH_ASSERT(pData);

    void* pLastElem = bohDynArrayPushBackDummy(pArray);
    pArray->pElemCopyFunc(pLastElem, pData);

    return pLastElem;
}


void* bohDynArrayAt(bohDynArray* pArray, size_t index)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    BOH_ASSERT(index < pArray->size);

    return BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArray->pData, index, pArray->elementSize);
}


const void* bohDynArrayAtConst(const bohDynArray *pArray, size_t index)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    BOH_ASSERT(index < pArray->size);

    return BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArray->pData, index, pArray->elementSize);
}


const void* bohDynArrayGetDataConst(const bohDynArray *pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    return pArray->pData;
}


void* bohDynArrayGetData(bohDynArray *pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    return pArray->pData;
}


size_t bohDynArrayGetSize(const bohDynArray* pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    return pArray->size;
}

size_t bohDynArrayGetCapacity(const bohDynArray* pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    return pArray->capacity;
}


bool bohDynArrayIsEmpty(const bohDynArray* pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    return pArray->size == 0;
}


size_t bohDynArrayGetMemorySize(const bohDynArray* pArray)
{
    BOH_ASSERT(bohDynArrayIsValid(pArray));
    return pArray->capacity * pArray->elementSize;
}


bool bohDynArrayIsValid(const bohDynArray* pArray)
{
    return pArray && pArray->elementSize > 0;
}


bohDynArray* bohDynArrayAssign(bohDynArray* pDst, const bohDynArray* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    if (bohDynArrayIsValid(pDst)) {
        bohDynArrayDestroy(pDst);
    }

    *pDst = bohDynArrayCreate(pSrc->elementSize, pSrc->pElemDefContr, pSrc->pElemDestr, pSrc->pElemCopyFunc);

    bohDynArrayReserve(pDst, pSrc->size);

    for (size_t i = 0; i < pSrc->size; ++i) {
        bohDynArrayPushBack(pDst, bohDynArrayAtConst(pSrc, i));
    }

    return pDst;
}


bohDynArray* bohDynArrayMove(bohDynArray* pDst, bohDynArray* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    if (bohDynArrayIsValid(pDst)) {
        bohDynArrayDestroy(pDst);
    }

    memcpy_s(pDst, sizeof(bohDynArray), pSrc, sizeof(bohDynArray));
    memset(pSrc, 0, sizeof(bohDynArray));

    return pDst;
}
