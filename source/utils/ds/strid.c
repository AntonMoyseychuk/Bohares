#include "pch.h"
#include "strid.h"

#include "string.h"
#include "string_view.h"

#include "hash.h"

#include "dyn_array.h"


static const uint64_t BOH_AVERAGE_STR_SIZE = 32;
static const uint64_t BOH_PREALLOCATED_STR_IDS_COUNT = 4096;
static const uint64_t BOH_PREALLOCATED_STORAGE_SIZE = BOH_PREALLOCATED_STR_IDS_COUNT * BOH_AVERAGE_STR_SIZE;
static const uint64_t BOH_INVALID_STR_ID_HASH = UINT64_MAX;


typedef struct HashToStrIDDataLoactionIdx
{
    uint64_t hash;
    uint64_t index;
} bohHashToLocationIdxMapping;

static void bohHashToLocationIdxMappingDestroy(bohHashToLocationIdxMapping* pMapping);
static void bohHashToLocationIdxMappingCreateInPlace(bohHashToLocationIdxMapping* pMapping, uint64_t hash, uint64_t index);

static bohHashToLocationIdxMapping* bohHashToLocationIdxMappingAssign(bohHashToLocationIdxMapping* pDst, const bohHashToLocationIdxMapping* pSrc);


typedef struct StrIDDataLocation
{
    size_t offset;
    size_t length;
} bohStrIDDataLocation;

static void bohStrIDDataLocationDestroy(bohStrIDDataLocation* pLocation);
static void bohStrIDDataLocationCreateInPlace(bohStrIDDataLocation* pLocation, size_t offset, size_t length);
static bohStrIDDataLocation* bohStrIDDataLocationAssign(bohStrIDDataLocation* pDst, const bohStrIDDataLocation* pSrc);


static void bohHashToLocationIdxMappingDestroy(bohHashToLocationIdxMapping* pMapping)
{
    BOH_ASSERT(pMapping);

    pMapping->hash = BOH_INVALID_STR_ID_HASH;
    pMapping->index = UINT64_MAX;
}


static void bohHashToLocationIdxMappingCreateInPlace(bohHashToLocationIdxMapping* pMapping, uint64_t hash, uint64_t index)
{
    BOH_ASSERT(pMapping);

    pMapping->hash = hash;
    pMapping->index = index;
}


static bohHashToLocationIdxMapping* bohHashToLocationIdxMappingAssign(bohHashToLocationIdxMapping* pDst, const bohHashToLocationIdxMapping* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->hash = pSrc->hash;
    pDst->index = pSrc->index;

    return pDst;
}


static void HashToStrIDDataLoactionIdxDefConst(void* pElement)
{
    bohHashToLocationIdxMappingCreateInPlace((bohHashToLocationIdxMapping*)pElement, BOH_INVALID_STR_ID_HASH, 0xffffffffffffffff);
}


static void HashToStrIDDataLoactionIdxDestr(void* pElement)
{
    bohHashToLocationIdxMappingDestroy((bohHashToLocationIdxMapping*)pElement);
}


static void HashToStrIDDataLoactionIdxCopy(void* pDst, const void* pSrc)
{
    bohHashToLocationIdxMappingAssign((bohHashToLocationIdxMapping*)pDst, (const bohHashToLocationIdxMapping*)pSrc);
}


static void bohStrIDDataLocationDestroy(bohStrIDDataLocation* pLocation)
{
    BOH_ASSERT(pLocation);

    pLocation->offset = 0;
    pLocation->length = 0;
}


static void bohStrIDDataLocationCreateInPlace(bohStrIDDataLocation* pLocation, size_t offset, size_t length)
{
    BOH_ASSERT(pLocation);

    pLocation->offset = offset;
    pLocation->length = length;
}


static bohStrIDDataLocation* bohStrIDDataLocationAssign(bohStrIDDataLocation* pDst, const bohStrIDDataLocation* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->offset = pSrc->offset;
    pDst->length = pSrc->length;

    return pDst;
}


static void StrIDDataLocationDefConst(void* pElement)
{
    bohStrIDDataLocationCreateInPlace((bohStrIDDataLocation*)pElement, 0, 0);
}


static void StrIDDataLocationDestr(void* pElement)
{
    bohStrIDDataLocationDestroy((bohStrIDDataLocation*)pElement);
}


static void StrIDDataLocationCopy(void* pDst, const void* pSrc)
{
    bohStrIDDataLocationAssign((bohStrIDDataLocation*)pDst, (const bohStrIDDataLocation*)pSrc);
}


typedef struct StrIDDataStorage
{
    bohDynArray hashToDataLocationIdx;
    bohDynArray strDataLocations;
    bohDynArray dataStorage;
        
    uint64_t lastAllocatedID;
    size_t size;
} bohStrIDDataStorage;


static uint64_t bohStrIDDataStorageFindLocationIdx(const bohStrIDDataStorage* pStorage, uint64_t hash)
{   
    const size_t mappingsCount = bohDynArrayGetSize(&pStorage->hashToDataLocationIdx);
    for (size_t i = 0; i < mappingsCount; ++i) {
        const bohHashToLocationIdxMapping* pMapping = BOH_DYN_ARRAY_AT_CONST(bohHashToLocationIdxMapping, &pStorage->hashToDataLocationIdx, i);
        
        if (pMapping->hash == hash) {
            return pMapping->index;
        }
    }

    return UINT64_MAX;
}


static void bohStrIDDataStorageDestroy(bohStrIDDataStorage* pStorage)
{
    BOH_ASSERT(pStorage);

    bohDynArrayDestroy(&pStorage->hashToDataLocationIdx);
    bohDynArrayDestroy(&pStorage->strDataLocations);
    bohDynArrayDestroy(&pStorage->dataStorage);

    pStorage->lastAllocatedID = BOH_INVALID_STR_ID_HASH;
    pStorage->size = 0;
}


static void bohStrIDDataStorageCreateInPlace(bohStrIDDataStorage* pStorage)
{
    BOH_ASSERT(pStorage);

    pStorage->hashToDataLocationIdx = BOH_DYN_ARRAY_CREATE(bohHashToLocationIdxMapping, HashToStrIDDataLoactionIdxDefConst, 
        HashToStrIDDataLoactionIdxDestr, HashToStrIDDataLoactionIdxCopy);
    bohDynArrayReserve(&pStorage->hashToDataLocationIdx, BOH_PREALLOCATED_STR_IDS_COUNT);

    pStorage->strDataLocations = BOH_DYN_ARRAY_CREATE(bohStrIDDataLocation, StrIDDataLocationDefConst, 
        StrIDDataLocationDestr, StrIDDataLocationCopy);
    bohDynArrayReserve(&pStorage->strDataLocations, BOH_PREALLOCATED_STR_IDS_COUNT);

    pStorage->dataStorage = bohDynArrayCreateUI8();
    bohDynArrayResize(&pStorage->dataStorage, BOH_PREALLOCATED_STORAGE_SIZE);

    pStorage->lastAllocatedID = BOH_INVALID_STR_ID_HASH;
    pStorage->size = 0;
}


static bool bohStrIDDataStorageIsExist(const bohStrIDDataStorage* pStorage, uint64_t id)
{
    BOH_ASSERT(pStorage);
    return bohStrIDDataStorageFindLocationIdx(pStorage, id) != UINT64_MAX;
}


static uint64_t bohStrIDDataStorageStoreStrViewPtr(bohStrIDDataStorage* pStorage, const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    const uint64_t id = bohHashStringView(pStrView);

    if (!bohStrIDDataStorageIsExist(pStorage, id)) {
        bohStrIDDataLocation* pNewLocation = bohDynArrayPushBackDummy(&pStorage->strDataLocations);

        bohHashToLocationIdxMapping* pHToI = bohDynArrayPushBackDummy(&pStorage->hashToDataLocationIdx);
        bohHashToLocationIdxMappingCreateInPlace(pHToI, id, bohDynArrayGetSize(&pStorage->strDataLocations) - 1);
            
        pNewLocation->length = bohStringViewGetSize(pStrView) + 1; // including null terminator

        if (pStorage->lastAllocatedID != BOH_INVALID_STR_ID_HASH) {
            const size_t idx = bohStrIDDataStorageFindLocationIdx(pStorage, pStorage->lastAllocatedID);
            BOH_ASSERT(idx != UINT64_MAX);

            const bohStrIDDataLocation* pLastLocation = bohDynArrayAtConst(&pStorage->strDataLocations, idx);

            pNewLocation->offset = pLastLocation->offset + pLastLocation->length;
        }

        const uint64_t newSize = pNewLocation->offset + pNewLocation->length;
            
        if (newSize > bohDynArrayGetSize(&pStorage->dataStorage)) {
            BOH_ASSERT_FAIL("StrID global data overflow");
            bohDynArrayResize(&pStorage->dataStorage, newSize * 2ull);
        }

        memcpy_s(BOH_DYN_ARRAY_GET_DATA(char, &pStorage->dataStorage) + pNewLocation->offset, pNewLocation->length - 1, 
            bohStringViewGetData(pStrView), pNewLocation->length - 1);

        pStorage->size = newSize;
        pStorage->lastAllocatedID = id;
    }

    return id;
}


static uint64_t bohStrIDDataStorageStore(bohStrIDDataStorage* pStorage, const char* pCStr)
{
    const bohStringView strView = bohStringViewCreateConstCStr(pCStr);
    return bohStrIDDataStorageStoreStrViewPtr(pStorage, &strView);
}


static uint64_t bohStrIDDataStorageStoreString(bohStrIDDataStorage* pStorage, const bohString* pStr)
{
    const bohStringView strView = bohStringViewCreateConstString(pStr);
    return bohStrIDDataStorageStoreStrViewPtr(pStorage, &strView);
}


const char* bohStrIDDataStorageLoad(const bohStrIDDataStorage* pStorage, uint64_t id)
{
    const uint64_t index = bohStrIDDataStorageFindLocationIdx(pStorage, id);

    if (index != UINT64_MAX) {
        const bohStrIDDataLocation* pLocation = BOH_DYN_ARRAY_AT_CONST(bohStrIDDataLocation, &pStorage->strDataLocations, index);
        return BOH_DYN_ARRAY_GET_DATA_CONST(char, &pStorage->dataStorage) + pLocation->offset;
    }

    return "";
}


uint64_t bohStrIDDataStorageGetCapacity(const bohStrIDDataStorage* pStorage)
{
    BOH_ASSERT(pStorage);
    return bohDynArrayGetCapacity(&pStorage->dataStorage);
}


uint64_t bohStrIDDataStorageGetSize(const bohStrIDDataStorage* pStorage)
{
    BOH_ASSERT(pStorage);
    return pStorage->size;
}


static bohStrIDDataStorage s_storage;


void bohStrIDInitStorage(void)
{
    bohStrIDDataStorageCreateInPlace(&s_storage);
}


void bohStrIDTermStorage(void)
{
    bohStrIDDataStorageDestroy(&s_storage);
}


size_t bohStrIDGetStorageMemorySize(void)
{
    return bohDynArrayGetMemorySize(&s_storage.dataStorage) + bohDynArrayGetMemorySize(&s_storage.hashToDataLocationIdx)
        + bohDynArrayGetMemorySize(&s_storage.strDataLocations);
}


bohStrID bohStrIDCreate(void)
{
    bohStrID strID;

#if defined(BOH_DEBUG)
    strID.pStr = "";
#endif
    strID.ID = BOH_INVALID_STR_ID_HASH;

    return strID;
}


bohStrID bohStrIDCreateCStr(const char* pCStr)
{
    BOH_ASSERT(pCStr);

    bohStrID strID;
    strID.ID = bohStrIDDataStorageStore(&s_storage, pCStr);

#if defined(BOH_DEBUG)
    strID.pStr = bohStrIDDataStorageLoad(&s_storage, strID.ID);
#endif

    return strID;
}


bohStrID bohStrIDCreateString(const bohString* pString)
{
    BOH_ASSERT(pString);

    bohStrID strID;
    strID.ID = bohStrIDDataStorageStoreString(&s_storage, pString);

#if defined(BOH_DEBUG)
    strID.pStr = bohStrIDDataStorageLoad(&s_storage, strID.ID);
#endif

    return strID;
}


bohStrID bohStrIDCreateStringView(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    bohStrID strID;
    strID.ID = bohStrIDDataStorageStoreStrViewPtr(&s_storage, pStrView);

#if defined(BOH_DEBUG)
    strID.pStr = bohStrIDDataStorageLoad(&s_storage, strID.ID);
#endif

    return strID;
}


bohStrID* bohStrIDAssign(bohStrID* pDst, const bohStrID* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

#if defined(BOH_DEBUG)
    pDst->pStr = pSrc->pStr;
#endif

    pDst->ID = pSrc->ID;

    return pDst;
}


bohStrID* bohStrIDAssignCStr(bohStrID* pDst, const char* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->ID = bohStrIDDataStorageStore(&s_storage, pSrc);

#if defined(BOH_DEBUG)
    pDst->pStr = bohStrIDDataStorageLoad(&s_storage, pDst->ID);
#endif

    return pDst;
}


bohStrID* bohStrIDAssignString(bohStrID* pDst, const bohString* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->ID = bohStrIDDataStorageStoreString(&s_storage, pSrc);

#if defined(BOH_DEBUG)
    pDst->pStr = bohStrIDDataStorageLoad(&s_storage, pDst->ID);
#endif

    return pDst;
}


bohStrID* bohStrIDAssignStringView(bohStrID* pDst, const bohStringView* pSrc)
{
    BOH_ASSERT(pDst);
    BOH_ASSERT(pSrc);

    pDst->ID = bohStrIDDataStorageStoreStrViewPtr(&s_storage, pSrc);

#if defined(BOH_DEBUG)
    pDst->pStr = bohStrIDDataStorageLoad(&s_storage, pDst->ID);
#endif

    return pDst;
}


const char* bohStrIDGetCStr(const bohStrID* pStrID)
{
    BOH_ASSERT(pStrID);

#if defined(ENG_DEBUG)
    return pStrID->pStr;
#else
    return bohStrIDDataStorageLoad(&s_storage, pStrID->ID);
#endif
}


bool bohStrIDEqual(const bohStrID* pLeft, const bohStrID* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    return pLeft->ID == pRight->ID;
}


bool bohStrIDNotEqual(const bohStrID* pLeft, const bohStrID* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    return pLeft->ID != pRight->ID;
}


bool bohStrIDLess(const bohStrID* pLeft, const bohStrID* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    return pLeft->ID < pRight->ID;
}


bool bohStrIDGreater(const bohStrID* pLeft, const bohStrID* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    return pLeft->ID > pRight->ID;
}


bool bohStrIDLessEqual(const bohStrID* pLeft, const bohStrID* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    return pLeft->ID <= pRight->ID;
}


bool bohStrIDGreaterEqual(const bohStrID* pLeft, const bohStrID* pRight)
{
    BOH_ASSERT(pLeft);
    BOH_ASSERT(pRight);

    return pLeft->ID >= pRight->ID;
}


uint64_t bohStrIDGetID(const bohStrID* pStrID)
{
    BOH_ASSERT(pStrID);
    return pStrID->ID;
}


uint64_t bohStrIDGetHash(const bohStrID* pStrID)
{
    BOH_ASSERT(pStrID);
    return pStrID->ID;
}


bool bohStrIDIsValid(const bohStrID* pStrID)
{
    BOH_ASSERT(pStrID);
    return pStrID->ID != BOH_INVALID_STR_ID_HASH;
}
