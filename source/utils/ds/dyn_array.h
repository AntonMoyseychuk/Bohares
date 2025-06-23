#pragma once

#include <stdint.h>


typedef void (*bohDynArrElemDefConstr)(void* pElement);
typedef void (*bohDynArrElemDestr)(void* pElement);
typedef void (*bohDynArrElemCopyFunc)(void* pDst, const void* pSrc);


typedef struct DynArray
{
    void* pData;
    size_t size;
    size_t capacity;

    size_t elementSize;

    bohDynArrElemDefConstr pElemDefContr;
    bohDynArrElemDestr pElemDestr;
    bohDynArrElemCopyFunc pElemCopyFunc;
} bohDynArray;


bohDynArray bohDynArrayCreate(size_t elementSize, 
    const bohDynArrElemDefConstr pConstr, 
    const bohDynArrElemDestr pDestr, 
    const bohDynArrElemCopyFunc pCopyFunc);
void bohDynArrayDestroy(bohDynArray* pArray);

void bohDynArrayReserve(bohDynArray* pArray, size_t newCapacity);
void bohDynArrayResize(bohDynArray* pArray, size_t newSize);

void bohDynArrayClear(bohDynArray* pArray);

// Push back empty slot at the end of array and return pointer to it
void* bohDynArrayPushBackDummy(bohDynArray* pArray);
void* bohDynArrayPushBack(bohDynArray* pArray, const void* pData);

void* bohDynArrayAt(bohDynArray* pArray, size_t index);
const void* bohDynArrayAtConst(const bohDynArray* pArray, size_t index);

const void* bohDynArrayGetDataConst(const bohDynArray* pArray);
void* bohDynArrayGetData(bohDynArray* pArray);

size_t bohDynArrayGetSize(const bohDynArray* pArray);
size_t bohDynArrayGetCapacity(const bohDynArray* pArray);
bool bohDynArrayIsEmpty(const bohDynArray* pArray);

size_t bohDynArrayGetMemorySize(const bohDynArray* pArray);

bool bohDynArrayIsValid(const bohDynArray* pArray);

bohDynArray* bohDynArrayAssign(bohDynArray* pDst, const bohDynArray* pSrc);
bohDynArray* bohDynArrayMove(bohDynArray* pDst, bohDynArray* pSrc);


#define BOH_DYN_ARRAY_CREATE(ELEM_TYPE, ELEM_DEF_CONSTR, ELEM_DESTR, ELEM_COPY_FUNC) \
    bohDynArrayCreate(sizeof(ELEM_TYPE), ELEM_DEF_CONSTR, ELEM_DESTR, ELEM_COPY_FUNC)

#define BOH_DYN_ARRAY_AT(ELEM_TYPE, ARRAY_PTR, INDEX)       (ELEM_TYPE*)bohDynArrayAt(ARRAY_PTR, INDEX)
#define BOH_DYN_ARRAY_AT_CONST(ELEM_TYPE, ARRAY_PTR, INDEX) (const ELEM_TYPE*)bohDynArrayAtConst(ARRAY_PTR, INDEX)

#define BOH_DYN_ARRAY_GET_DATA(ELEM_TYPE, ARRAY_PTR)       (ELEM_TYPE*)bohDynArrayGetData(ARRAY_PTR)
#define BOH_DYN_ARRAY_GET_DATA_CONST(ELEM_TYPE, ARRAY_PTR) (const ELEM_TYPE*)bohDynArrayGetDataConst(ARRAY_PTR)
