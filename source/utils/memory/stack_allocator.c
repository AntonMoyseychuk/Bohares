#include "pch.h"

#include "stack_allocator.h"

#include "core.h"


#define BOH_ALIGN_UP(PTR, ALIGNMENT) ((void *)(((uintptr_t)(PTR) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1)))
#define BOH_ALIGN_DOWN(PTR, ALIGNMENT) ((void *)(((uintptr_t)(PTR)) & ~(uintptr_t)((ALIGNMENT) - 1)))


void bohStackAllocatorDestroy(bohStackAllocator* pAllocator)
{
    BOH_ASSERT(pAllocator);    

    free(pAllocator->pMemory);
    pAllocator->pMemory = NULL;
    pAllocator->topOffset = 0;
    pAllocator->capacity = 0;
}


bohStackAllocator bohStackAllocatorCreate(size_t capacity, bohStackAllocatorType type)
{
    bohStackAllocator allocator = {0};
    
    allocator.pMemory = (uint8_t*)malloc(capacity);
    BOH_ASSERT(allocator.pMemory);

    memset(allocator.pMemory, 0, capacity);

    allocator.capacity = capacity;
    allocator.type = type;
    allocator.topOffset = (type == BOH_STACK_ALLOCATOR_TYPE_FORWARD) ? 0 : capacity;

    return allocator;
}


void* bohStackAllocatorAlloc(bohStackAllocator* pAllocator, size_t size, size_t alignment)
{
    BOH_ASSERT(pAllocator);
    BOH_ASSERT(pAllocator->pMemory);
    BOH_ASSERT(size > 0);
    BOH_ASSERT(alignment > 0);
    
    uintptr_t base = (uintptr_t)pAllocator->pMemory;

    if (pAllocator->type == BOH_STACK_ALLOCATOR_TYPE_FORWARD) {
        const uintptr_t raw     = base + pAllocator->topOffset;
        const uintptr_t aligned = BOH_ALIGN_UP(raw, alignment);
        const size_t offset     = aligned + size - base;
        
        if (offset > pAllocator->capacity) {
            BOH_ASSERT_FAIL("Out of memory");
            return NULL;
        }

        pAllocator->topOffset = offset;
        return (void*)aligned;
    } else {
        const uintptr_t raw = base + pAllocator->topOffset - size;
        const uintptr_t aligned = BOH_ALIGN_DOWN(raw, alignment);
        
        if (aligned < base) {
            BOH_ASSERT_FAIL("Out of memory");
            return NULL;
        }
        
        pAllocator->topOffset = aligned - base;
        return (void*)aligned;
    }
}


void bohStackAllocatorFree(bohStackAllocator* pAllocator, size_t size, size_t alignment)
{
    BOH_ASSERT(pAllocator);
    BOH_ASSERT(pAllocator->pMemory);
    BOH_ASSERT(size > 0);
    BOH_ASSERT(alignment > 0);

    const size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    if (pAllocator->type == BOH_STACK_ALLOCATOR_TYPE_FORWARD) {
        BOH_ASSERT(pAllocator->topOffset >= alignedSize);
        pAllocator->topOffset -= alignedSize;
    } else {
        BOH_ASSERT(pAllocator->topOffset + alignedSize <= pAllocator->capacity);
        pAllocator->topOffset += alignedSize;
    }
}


size_t bohStackAllocatorGetCapacity(const bohStackAllocator* pAllocator)
{
    BOH_ASSERT(pAllocator);
    return pAllocator->capacity;
}


bool bohStackAllocatorIsForward(const bohStackAllocator* pAllocator)
{
    BOH_ASSERT(pAllocator);
    return pAllocator->type == BOH_STACK_ALLOCATOR_TYPE_FORWARD;
}
