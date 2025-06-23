#pragma once

#include <stdint.h>
#include <stdbool.h>


typedef struct StackAllocator
{
    uint8_t* pMemory;
    size_t topOffset;
    size_t capacity;
    int32_t addressGrowthDirection;
} bohStackAllocator;


// the offset occurs upward in addresses from 0 to capacity
bohStackAllocator bohStackAllocatorCreate(size_t capacity);
// the offset occurs downward in addresses from capacity - 1 to 0
bohStackAllocator bohStackAllocatorCreateReversed(size_t capacity);

void bohStackAllocatorDestroy(bohStackAllocator* pAllocator);

void* bohStackAllocatorAlloc(bohStackAllocator* pAllocator, size_t size, size_t alignment);
void bohStackAllocatorFree(bohStackAllocator* pAllocator, size_t size, size_t alignment);

size_t bohStackAllocatorGetTopOffset(const bohStackAllocator* pAllocator);
size_t bohStackAllocatorGetCapacity(const bohStackAllocator* pAllocator);

#define BOH_STACK_ALLOCATOR_ALLOC(STACK_ALLOC_PTR, TYPE) (TYPE*)bohStackAllocatorAlloc(STACK_ALLOC_PTR, sizeof(TYPE), _Alignof(TYPE))
#define BOH_STACK_ALLOCATOR_FREE(STACK_ALLOC_PTR, TYPE) bohStackAllocatorFree(STACK_ALLOC_PTR, sizeof(TYPE), _Alignof(TYPE))
