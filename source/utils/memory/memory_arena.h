#pragma once


typedef struct MemoryArena
{
    uint8_t* pMemory;
    size_t offset;
    size_t capacity;
} bohMemoryArena;


bohMemoryArena bohMemoryArenaCreate(size_t capacity);
void bohMemoryArenaDestroy(bohMemoryArena* pArena);

void* bohMemoryArenaAlloc(bohMemoryArena* pArena, size_t size, size_t alignment);

size_t bohMemoryArenaGetOffset(const bohMemoryArena* pArena);
size_t bohMemoryArenaGetCapacity(const bohMemoryArena* pArena);


#define BOH_MEMORY_ARENA_ALLOC(ARENA_PTR, TYPE) (TYPE*)bohMemoryArenaAlloc(ARENA_PTR, sizeof(TYPE), _Alignof(TYPE))
