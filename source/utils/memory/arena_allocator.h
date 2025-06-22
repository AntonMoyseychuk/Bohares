#pragma once


typedef struct ArenaAllocator
{
    uint8_t* pMemory;
    size_t offset;
    size_t capacity;
} bohArenaAllocator;


bohArenaAllocator bohArenaAllocatorCreate(size_t capacity);
void bohArenaAllocatorDestroy(bohArenaAllocator* pArena);

void* bohArenaAllocatorAlloc(bohArenaAllocator* pArena, size_t size, size_t alignment);

size_t bohArenaAllocatorGetOffset(const bohArenaAllocator* pArena);
size_t bohArenaAllocatorGetCapacity(const bohArenaAllocator* pArena);


#define BOH_ARENA_ALLOCATOR_ALLOC(ARENA_PTR, TYPE) (TYPE*)bohArenaAllocatorAlloc(ARENA_PTR, sizeof(TYPE), _Alignof(TYPE))
