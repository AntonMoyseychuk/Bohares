#include "pch.h"

#include "stack_allocator.h"

#include "core.h"


#define BOH_STACK_GROWTH_UP 1
#define BOH_STACK_GROWTH_DOWN -1

#define BOH_ALIGN_UP(PTR, ALIGNMENT) ((void *)(((uintptr_t)(PTR) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1)))


static bohStackAllocator bohStackAllocatorCreateDirectional(size_t capacity, bool isReversed)
{
    BOH_ASSERT(capacity > 0);

    bohStackAllocator allocator = {0};
    
    allocator.pMemory = malloc(capacity);
    memset(allocator.pMemory, 0, capacity);

    allocator.addressGrowthDirection = isReversed ? -1 : 1;

    allocator.topOffset = isReversed ? capacity : 0;
    allocator.capacity = capacity;

    return allocator;
}


bohStackAllocator bohStackAllocatorCreate(size_t capacity)
{
    return bohStackAllocatorCreateDirectional(capacity, false);
}


bohStackAllocator bohStackAllocatorCreateReversed(size_t capacity)
{
    return bohStackAllocatorCreateDirectional(capacity, true);
}


void bohStackAllocatorDestroy(bohStackAllocator* pAllocator)
{
    BOH_ASSERT(pAllocator);

    free(pAllocator->pMemory);
    pAllocator->pMemory = NULL;

    pAllocator->capacity = 0;
    pAllocator->topOffset = 0;
    pAllocator->addressGrowthDirection = 1;
}


void* bohStackAllocatorAlloc(bohStackAllocator* pAllocator, size_t size, size_t alignment)
{
    BOH_ASSERT(pAllocator);
    BOH_ASSERT(size > 0);
    BOH_ASSERT(alignment > 0);

    const size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    // Check if there is enough space for allocation
    if (pAllocator->addressGrowthDirection == BOH_STACK_GROWTH_UP) {
        BOH_ASSERT(pAllocator->topOffset + alignedSize <= pAllocator->capacity);
        
        void* pAlloc = pAllocator->pMemory + pAllocator->topOffset;
        pAllocator->topOffset += alignedSize;

        return BOH_ALIGN_UP(pAlloc, alignment);
    } else {
        BOH_ASSERT(pAllocator->topOffset >= alignedSize);

        pAllocator->topOffset -= alignedSize;
        void* pAlloc = pAllocator->pMemory + pAllocator->topOffset;

        return BOH_ALIGN_UP(pAlloc, alignment);
    }
}


void bohStackAllocatorFree(bohStackAllocator* pAllocator, size_t size, size_t alignment)
{
    BOH_ASSERT(pAllocator);
    BOH_ASSERT(size > 0);
    BOH_ASSERT(alignment > 0);

    const size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    if (pAllocator->addressGrowthDirection == BOH_STACK_GROWTH_UP) {
        pAllocator->topOffset -= alignedSize;
    } else {
        pAllocator->topOffset += alignedSize;
    }
}


size_t bohStackAllocatorGetTopOffset(const bohStackAllocator* pAllocator)
{
    BOH_ASSERT(pAllocator);
    return pAllocator->topOffset;
}


size_t bohStackAllocatorGetCapacity(const bohStackAllocator* pAllocator)
{
    BOH_ASSERT(pAllocator);
    return pAllocator->capacity;
}
