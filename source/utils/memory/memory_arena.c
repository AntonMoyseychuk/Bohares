#include "pch.h"

#include "memory_arena.h"

#include "core.h"

#include <stdlib.h>
#include <string.h>


static size_t bohAlignForward(size_t ptr, size_t alignment)
{
    const size_t modulo = ptr % alignment;
    return modulo == 0 ? ptr : ptr + (alignment - modulo);
}


bohMemoryArena bohMemoryArenaCreate(size_t capacity)
{
    bohMemoryArena arena = {0};

    arena.pMemory = malloc(capacity);
    memset(arena.pMemory, 0, capacity);

    arena.offset = 0;
    arena.capacity = capacity;

    return arena;
}


void bohMemoryArenaDestroy(bohMemoryArena* pArena)
{
    BOH_ASSERT(pArena);

    free(pArena->pMemory);
    pArena->pMemory = NULL;
    pArena->offset = 0;
    pArena->capacity = 0;
}


void* bohMemoryArenaAlloc(bohMemoryArena* pArena, size_t size, size_t alignment)
{
    BOH_ASSERT(pArena);

    const size_t current = bohAlignForward((size_t)(pArena->pMemory + pArena->offset), alignment);
    const size_t newOffset = current + size - (size_t)pArena->pMemory;

    BOH_ASSERT(newOffset <= pArena->capacity);
    
    pArena->offset = newOffset;
    
    return pArena->pMemory + (current - (size_t)pArena->pMemory);
}
