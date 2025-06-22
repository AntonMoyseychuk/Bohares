#include "pch.h"

#include "arena_allocator.h"

#include "core.h"

#include <stdlib.h>
#include <string.h>


static size_t bohAlignForward(size_t ptr, size_t alignment)
{
    const size_t modulo = ptr % alignment;
    return modulo == 0 ? ptr : ptr + (alignment - modulo);
}


bohArenaAllocator bohArenaAllocatorCreate(size_t capacity)
{
    bohArenaAllocator arena = {0};

    arena.pMemory = malloc(capacity);
    memset(arena.pMemory, 0, capacity);

    arena.offset = 0;
    arena.capacity = capacity;

    return arena;
}


void bohArenaAllocatorDestroy(bohArenaAllocator* pArena)
{
    BOH_ASSERT(pArena);

    free(pArena->pMemory);
    pArena->pMemory = NULL;
    pArena->offset = 0;
    pArena->capacity = 0;
}


void* bohArenaAllocatorAlloc(bohArenaAllocator* pArena, size_t size, size_t alignment)
{
    BOH_ASSERT(pArena);

    const size_t current = bohAlignForward((size_t)(pArena->pMemory + pArena->offset), alignment);
    const size_t newOffset = current + size - (size_t)pArena->pMemory;

    BOH_ASSERT(newOffset <= pArena->capacity);
    
    pArena->offset = newOffset;
    
    return pArena->pMemory + (current - (size_t)pArena->pMemory);
}


size_t bohArenaAllocatorGetOffset(const bohArenaAllocator* pArena)
{
    BOH_ASSERT(pArena);
    return pArena->offset;
}


size_t bohArenaAllocatorGetCapacity(const bohArenaAllocator* pArena)
{
    BOH_ASSERT(pArena);
    return pArena->capacity;
}
