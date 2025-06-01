#include "pch.h"

#include "dyn_array.h"


#define BOH_GET_DYN_ARRAY_ELEMENT_PTR(DATA, INDEX, ELEM_SIZE) (void*)((uint8_t*)(DATA) + (INDEX) * (ELEM_SIZE))


bohDynArray bohDynArrayCreate(size_t elementSize, 
    const bohDynArrElemDefConstr pConstr, 
    const bohDynArrElemDestr pDestr, 
    const bohDynArrElemCopyFunc pCopyFunc)
{
    assert(elementSize > 0);
    assert(pConstr);
    assert(pDestr);
    assert(pCopyFunc);

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
    assert(bohDynArrayIsValid(pArray));

    bohDynArrayResize(pArray, 0);

    free(pArray->pData);
    pArray->capacity = 0;

    pArray->elementSize = 0;
    pArray->pElemDefContr = NULL;
    pArray->pElemDestr = NULL;
    pArray->pElemCopyFunc = NULL;
}


void bohDynArrayReserve(bohDynArray* pArray, size_t newCapacity)
{
    assert(bohDynArrayIsValid(pArray));

    if (newCapacity <= pArray->capacity) {
        return;
    }

    const size_t newCapacityInBytes = newCapacity * pArray->elementSize;

    void* pNewBuffer = malloc(newCapacityInBytes);
    assert(pNewBuffer);

    memset(pNewBuffer, 0, newCapacityInBytes);

    const size_t size = pArray->size;
    void* pOldBuffer = pArray->pData;

    const bohDynArrElemCopyFunc ElemCopyFunc = pArray->pElemCopyFunc;
    const size_t elemSize = pArray->elementSize;

    for (size_t i = 0; i < size; ++i) {
        void* pDst = BOH_GET_DYN_ARRAY_ELEMENT_PTR(pNewBuffer, i, elemSize);
        const void* pSrc = BOH_GET_DYN_ARRAY_ELEMENT_PTR(pOldBuffer, i, elemSize);

        ElemCopyFunc(pDst, pSrc);
    }
    
    pArray->pData = pNewBuffer;
    pArray->capacity = newCapacity;

    const bohDynArrElemDestr ElemDestr = pArray->pElemDestr;

    for (size_t i = 0; i < size; ++i) {
        ElemDestr(BOH_GET_DYN_ARRAY_ELEMENT_PTR(pOldBuffer, i, elemSize));
    }

    free(pOldBuffer);
}


void bohDynArrayResize(bohDynArray* pArray, size_t newSize)
{
    assert(bohDynArrayIsValid(pArray));

    const size_t oldSize = pArray->size;

    if (newSize == oldSize) {
        return;
    }

    void* pArrData = pArray->pData;
    const size_t elemSize = pArray->elementSize;

    if (newSize < oldSize) {
        const bohDynArrElemDestr ElemDestructor = pArray->pElemDestr;

        for (size_t i = newSize; i < oldSize; ++i) {
            ElemDestructor(BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArrData, i, elemSize));
        }

        pArray->size = newSize;

        return;
    }

    const bohDynArrElemDefConstr ElemConstructor = pArray->pElemDefContr;

    bohDynArrayReserve(pArray, newSize);
    for (size_t i = oldSize; i < newSize; ++i) {
        ElemConstructor(BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArrData, i, elemSize));
    }

    pArray->size = newSize;
}


void bohDynArrayClear(bohDynArray* pArray)
{
    assert(bohDynArrayIsValid(pArray));
    bohDynArrayResize(pArray, 0);
}


void* bohDynArrayPushBackDummy(bohDynArray *pArray)
{
    assert(bohDynArrayIsValid(pArray));

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
    assert(bohDynArrayIsValid(pArray));
    assert(pData);

    void* pLastElem = bohDynArrayPushBackDummy(pArray);
    pArray->pElemCopyFunc(pLastElem, pData);

    return pLastElem;
}


void* bohDynArrayAt(bohDynArray* pArray, size_t index)
{
    assert(bohDynArrayIsValid(pArray));
    assert(index < pArray->size);

    return BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArray->pData, index, pArray->elementSize);
}


const void* bohDynArrayAtConst(const bohDynArray *pArray, size_t index)
{
    assert(bohDynArrayIsValid(pArray));
    assert(index < pArray->size);

    return BOH_GET_DYN_ARRAY_ELEMENT_PTR(pArray->pData, index, pArray->elementSize);
}


size_t bohDynArrayGetSize(const bohDynArray* pArray)
{
    assert(bohDynArrayIsValid(pArray));
    return pArray->size;
}

size_t bohDynArrayGetCapacity(const bohDynArray* pArray)
{
    assert(bohDynArrayIsValid(pArray));
    return pArray->capacity;
}


bool bohDynArrayIsEmpty(const bohDynArray* pArray)
{
    assert(bohDynArrayIsValid(pArray));
    return pArray->size == 0;
}


size_t bohDynArrayGetMemorySize(const bohDynArray* pArray)
{
    assert(bohDynArrayIsValid(pArray));
    return pArray->size * pArray->elementSize;
}


bool bohDynArrayIsValid(const bohDynArray* pArray)
{
    return pArray && pArray->elementSize > 0 && pArray->pElemDefContr && pArray->pElemDestr && pArray->pElemCopyFunc;
}
