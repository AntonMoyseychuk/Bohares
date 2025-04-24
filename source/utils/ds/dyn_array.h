#pragma once


typedef void (*bohDynArrElemCopyFunc)(void* pDst, const void* pSrc);
typedef void (*bohDynArrElemDefaultConstructor)(void* pElement);
typedef void (*bohDynArrElemDestructor)(void* pElement);


typedef struct DynArray
{
    void* pData;
    size_t size;
    size_t capacity;

    size_t elementSize;

    bohDynArrElemDefaultConstructor pElemDefContr;
    bohDynArrElemDestructor pElemDestr;
    bohDynArrElemCopyFunc pElemCopyFunc;
} bohDynArray;


bohDynArray bohDynArrayCreate(size_t elementSize, 
    const bohDynArrElemDefaultConstructor pConstr, 
    const bohDynArrElemDestructor pDestr, 
    const bohDynArrElemCopyFunc pCopyFunc);
void bohDynArrayDestroy(bohDynArray* pArray);

void bohDynArrayReserve(bohDynArray* pArray, size_t newCapacity);
void bohDynArrayResize(bohDynArray* pArray, size_t newSize);

void bohDynArrayPushBack(bohDynArray* pArray, const void* pData);

void* bohDynArrayAt(bohDynArray* pArray, size_t index);
const void* bohDynArrayAtConst(const bohDynArray* pArray, size_t index);

size_t bohDynArrayGetSize(const bohDynArray* pArray);
size_t bohDynArrayGetCapacity(const bohDynArray* pArray);
bool bohDynArrayIsEmpty(const bohDynArray* pArray);

size_t bohDynArrayGetMemorySize(const bohDynArray* pArray);

bool bohDynArrayIsValid(const bohDynArray* pArray);


#define BOH_DYN_ARRAY_CREATE(ELEM_TYPE, ELEM_DEF_CONSTR, ELEM_DESTR, ELEM_COPY_FUNC) \
    bohDynArrayCreate(sizeof(ELEM_TYPE), ELEM_DEF_CONSTR, ELEM_DESTR, ELEM_COPY_FUNC)
