#include "pch.h"

#include "hash.h"
#include "core.h"


uint64_t bohHashCStr(const char* pStr, size_t size)
{
    const bohStringView strView = bohStringViewCreateConstCStrSized(pStr, size);
    return bohHashStringView(&strView);
}


uint64_t bohHashString(const bohString* pString)
{
    const bohStringView strView = bohStringViewCreateConstString(pString);
    return bohHashStringView(&strView);
}


// MurmurHash3 
uint64_t bohHashStringView(const bohStringView* pStrView)
{
    BOH_ASSERT(pStrView);

    static const uint64_t seed = 0xc70f6907UL;

    if (bohStringViewIsEmpty(pStrView)) {
        return UINT64_C(0);
    }

    uint64_t hash = seed;

    const size_t size = bohStringViewGetSize(pStrView);
    
    for (size_t i = 0; i < size; ++i) {
        hash ^= (uint64_t)bohStringViewAt(pStrView, i);
        hash *= 0x100000001b3;
        hash ^= hash >> 33;
    }

    return hash;
}


uint64_t bohHashMem(const void *data, size_t size)
{
    if (!data || size == 0) {
        return 0;
    }

    const uint64_t* ptr = (const uint64_t*)data;

    uint64_t hash = 0;
    int64_t remaining = size / sizeof(uint64_t);
    
    while (remaining > 0) {
        hash ^= *ptr++;
        hash *= 1103515245ull;
        hash ^= *ptr++;
        hash *= 1103515245ull;
        remaining -= (int64_t)sizeof(uint64_t);
    }

    const uint8_t* tail = (const uint8_t*)ptr;
    const size_t tailLength = size % sizeof(uint64_t);

    for (size_t i = 0; i < tailLength; ++i) {
        hash ^= ((uint64_t)tail[i]) << (i * 8);
        hash *= 1103515245ull;
    }

    return hash;
}
