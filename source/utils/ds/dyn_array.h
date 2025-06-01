#pragma once


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

size_t bohDynArrayGetSize(const bohDynArray* pArray);
size_t bohDynArrayGetCapacity(const bohDynArray* pArray);
bool bohDynArrayIsEmpty(const bohDynArray* pArray);

size_t bohDynArrayGetMemorySize(const bohDynArray* pArray);

bool bohDynArrayIsValid(const bohDynArray* pArray);


#define BOH_DYN_ARRAY_CREATE(ELEM_TYPE, ELEM_DEF_CONSTR, ELEM_DESTR, ELEM_COPY_FUNC) \
    bohDynArrayCreate(sizeof(ELEM_TYPE), ELEM_DEF_CONSTR, ELEM_DESTR, ELEM_COPY_FUNC)
