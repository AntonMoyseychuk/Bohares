#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


typedef enum StackAllocatorType
{
    BOH_STACK_ALLOCATOR_TYPE_FORWARD,
    BOH_STACK_ALLOCATOR_TYPE_REVERSE
} bohStackAllocatorType;

typedef struct {
    uint8_t* pMemory;
    size_t topOffset;
    size_t capacity;
    bohStackAllocatorType type;
} bohStackAllocator;


void bohStackAllocatorDestroy(bohStackAllocator* allocator);

bohStackAllocator bohStackAllocatorCreate(size_t capacity, bohStackAllocatorType type);

void* bohStackAllocatorAlloc(bohStackAllocator* allocator, size_t size, size_t alignment);
void bohStackAllocatorFree(bohStackAllocator* allocator, size_t size, size_t alignment);

size_t bohStackAllocatorGetCapacity(const bohStackAllocator* pAllocator);

bool bohStackAllocatorIsForward(const bohStackAllocator* pAllocator);


#define BOH_STACK_ALLOCATOR_ALLOC(STACK_ALLOC_PTR, TYPE) (TYPE*)bohStackAllocatorAlloc(STACK_ALLOC_PTR, sizeof(TYPE), _Alignof(TYPE))
#define BOH_STACK_ALLOCATOR_FREE(STACK_ALLOC_PTR, TYPE) bohStackAllocatorFree(STACK_ALLOC_PTR, sizeof(TYPE), _Alignof(TYPE))
