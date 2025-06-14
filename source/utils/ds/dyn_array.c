#include "pch.h"

#include "core.h"

#include "dyn_array.h"


#define BOH_CONSTRUCT_BASE_TYPE_DEFAULT_CONSTRUCTOR_NAME(TYPE) bohDynArrElemDefConstr_##TYPE##_
#define BOH_CONSTRUCT_BASE_TYPE_DESTRUCTOR_NAME(TYPE) bohDynArrElemDestr_##TYPE##_
#define BOH_CONSTRUCT_BASE_TYPE_COPY_FUNC_NAME(TYPE) bohDynArrElemCopyFunc_##TYPE##_

#define BOH_DECLARE_BASE_TYPE_DEFAULT_CONSTRUCT(TYPE)                           \
    void BOH_CONSTRUCT_BASE_TYPE_DEFAULT_CONSTRUCTOR_NAME(TYPE)(void* pElement) \
    {                                                                           \
        *((TYPE*)pElement) = (TYPE)0;                                           \
    }

#define BOH_DECLARE_BASE_TYPE_DESTRUCTOR(TYPE)                          \
    void BOH_CONSTRUCT_BASE_TYPE_DESTRUCTOR_NAME(TYPE)(void* pElement)  \
    {                                                                   \
        *((TYPE*)pElement) = (TYPE)0;                                   \
    }

#define BOH_DECLARE_BASE_TYPE_COPY_FUNC(TYPE)                                       \
    void BOH_CONSTRUCT_BASE_TYPE_COPY_FUNC_NAME(TYPE)(void* pDst, const void* pSrc) \
    {                                                                               \
        *((TYPE*)pDst) = *((const TYPE*)pSrc);                                      \
    }

#define BOH_DECALRE_BASE_TYPE_ARR_FUNCS(TYPE)       \
    BOH_DECLARE_BASE_TYPE_DEFAULT_CONSTRUCT(TYPE)   \
    BOH_DECLARE_BASE_TYPE_DESTRUCTOR(TYPE)          \
    BOH_DECLARE_BASE_TYPE_COPY_FUNC(TYPE)

#define BOH_GET_BASE_TYPE_DEF_CONSTR(TYPE) BOH_CONSTRUCT_BASE_TYPE_DEFAULT_CONSTRUCTOR_NAME(TYPE)
#define BOH_GET_BASE_TYPE_DESTR(TYPE)      BOH_CONSTRUCT_BASE_TYPE_DESTRUCTOR_NAME(TYPE)
#define BOH_GET_BASE_TYPE_COPY_FUNC(TYPE)  BOH_CONSTRUCT_BASE_TYPE_COPY_FUNC_NAME(TYPE)


BOH_DECALRE_BASE_TYPE_ARR_FUNCS(int8_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(uint8_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(int16_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(uint16_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(int32_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(uint32_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(int64_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(uint64_t)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(float)
BOH_DECALRE_BASE_TYPE_ARR_FUNCS(double)


#define BOH_GET_DYN_ARRAY_ELEMENT_PTR(DATA, INDEX, ELEM_SIZE) (void*)((uint8_t*)(DATA) + (INDEX) * (ELEM_SIZE))


bohDynArray bohDynArrayCreate(size_t elementSize, 
    const bohDynArrElemDefConstr pConstr, 
    const bohDynArrElemDestr pDestr, 
    const bohDynArrElemCopyFunc pCopyFunc)
{
    BOH_ASSERT(elementSize > 0);
    BOH_ASSERT(pConstr);
    BOH_ASSERT(pDestr);
    BOH_ASSERT(pCopyFunc);

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

    const size_t newCapacityInBytes = newCapacity * pArray->elementSize;

    void* pNewBuffer = malloc(newCapacityInBytes);
    BOH_ASSERT(pNewBuffer);

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
    BOH_ASSERT(bohDynArrayIsValid(pArray));

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
    return pArray->size * pArray->elementSize;
}


bool bohDynArrayIsValid(const bohDynArray* pArray)
{
    return pArray && pArray->elementSize > 0 && pArray->pElemDefContr && pArray->pElemDestr && pArray->pElemCopyFunc;
}


bohDynArray* bohDynArrayAssign(bohDynArray* pDst, const bohDynArray* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    if (bohDynArrayIsValid(pDst)) {
        bohDynArrayDestroy(pDst);
    }

    *pDst = bohDynArrayCreate(pSrc->elementSize, pSrc->pElemDefContr, pSrc->pElemDestr, pSrc->pElemCopyFunc);

    bohDynArrayReserve(pDst, pSrc->capacity);

    for (size_t i = 0; i < pSrc->size; ++i) {
        bohDynArrayPushBack(pDst, bohDynArrayAtConst(pSrc, i));
    }

    return pDst;
}


bohDynArray* bohDynArrayMove(bohDynArray* pDst, bohDynArray* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    bohDynArrayDestroy(pDst);

    memcpy_s(pDst, sizeof(bohDynArray), pSrc, sizeof(bohDynArray));
    memset(pSrc, 0, sizeof(bohDynArray));

    return pDst;
}


bohDynArray bohDynArrayCreateI8(void)
{
    return BOH_DYN_ARRAY_CREATE(int8_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(int8_t), BOH_GET_BASE_TYPE_DESTR(int8_t), BOH_GET_BASE_TYPE_COPY_FUNC(int8_t));
}


bohDynArray bohDynArrayCreateUI8(void)
{
    return BOH_DYN_ARRAY_CREATE(uint8_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(uint8_t), BOH_GET_BASE_TYPE_DESTR(uint8_t), BOH_GET_BASE_TYPE_COPY_FUNC(uint8_t));
}


bohDynArray bohDynArrayCreateI16(void)
{
    return BOH_DYN_ARRAY_CREATE(int16_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(int16_t), BOH_GET_BASE_TYPE_DESTR(int16_t), BOH_GET_BASE_TYPE_COPY_FUNC(int16_t));
}


bohDynArray bohDynArrayCreateUI16(void)
{
    return BOH_DYN_ARRAY_CREATE(uint16_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(uint16_t), BOH_GET_BASE_TYPE_DESTR(uint16_t), BOH_GET_BASE_TYPE_COPY_FUNC(uint16_t));
}


bohDynArray bohDynArrayCreateI32(void)
{
    return BOH_DYN_ARRAY_CREATE(int32_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(int32_t), BOH_GET_BASE_TYPE_DESTR(int32_t), BOH_GET_BASE_TYPE_COPY_FUNC(int32_t));
}


bohDynArray bohDynArrayCreateUI32(void)
{
    return BOH_DYN_ARRAY_CREATE(uint32_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(uint32_t), BOH_GET_BASE_TYPE_DESTR(uint32_t), BOH_GET_BASE_TYPE_COPY_FUNC(uint32_t));
}


bohDynArray bohDynArrayCreateI64(void)
{
    return BOH_DYN_ARRAY_CREATE(int64_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(int64_t), BOH_GET_BASE_TYPE_DESTR(int64_t), BOH_GET_BASE_TYPE_COPY_FUNC(int64_t));
}


bohDynArray bohDynArrayCreateUI64(void)
{
    return BOH_DYN_ARRAY_CREATE(uint64_t, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(uint64_t), BOH_GET_BASE_TYPE_DESTR(uint64_t), BOH_GET_BASE_TYPE_COPY_FUNC(uint64_t));
}


bohDynArray bohDynArrayCreateFloat(void)
{
    return BOH_DYN_ARRAY_CREATE(float, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(float), BOH_GET_BASE_TYPE_DESTR(float), BOH_GET_BASE_TYPE_COPY_FUNC(float));
}


bohDynArray bohDynArrayCreateDouble(void)
{
    return BOH_DYN_ARRAY_CREATE(double, 
        BOH_GET_BASE_TYPE_DEF_CONSTR(double), BOH_GET_BASE_TYPE_DESTR(double), BOH_GET_BASE_TYPE_COPY_FUNC(double));
}
