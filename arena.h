/*
    Create implementation: #define ARENA_IMPLEMENTATION before including this file
*/

#ifndef ARENA_H
#define ARENA_H

#ifdef __cplusplus
extern "C" {
#endif

// Types
#include "stdint.h"

#ifndef u64
#define u64 uint64_t
#endif

#ifndef u16
#define u16 uint16_t
#endif

// If you need custom allocators for inner arena buffers, define ARENA_CUSTOM_ALLOC
// and all three allocation methods like this before including arena.h:
// #define ARENA_CUSTOM_ALLOC
// #define Arena_Malloc(size) mi_malloc(size)
// #define Arena_Free(data) mi_free(data)
// #define Arena_Realloc(data, size) mi_realloc(data, size)
#ifndef ARENA_CUSTOM_ALLOC
    #include <stdlib.h>

    #ifndef Arena_Malloc
        #define Arena_Malloc(size) malloc(size)
    #endif

    #ifndef Arena_Free
        #define Arena_Free(data) free(data)
    #endif

    #ifndef Arena_Realloc
        #define Arena_Realloc(data, size) realloc(data, size)
    #endif
#endif

#include <assert.h>

typedef struct Arena {
    char   *data;
    u64     capacity;
    u64     allocated;
} Arena;

#define push_array(arena, type, count) (type*)(arena_alloc(arena, sizeof(type) * count))
#define push_struct(arena, type) push_array(arena, type, 1)
#define push_array_aligned(arena, type, count, align) (type*)(arena_alloc_aligned(arena, sizeof(type) * count, align))
#define push_struct_aligned(arena, type, align) (type*)(arena_alloc_aligned(arena, sizeof(type), align))

static        Arena *make_arena(u64 size);
inline static void  *arena_alloc(Arena *arena, u64 size);
inline static void  *arena_alloc_aligned(Arena *arena, u64 size, u16 align);
inline static void   arena_flush(Arena *arena);
inline static void   arena_free(Arena *arena);
inline static u16    arena_realloc(Arena *arena, u64 new_size);

#ifdef ARENA_IMPLEMENTATION

static Arena *make_arena(u64 size) {
    if(size == 0) {
        return NULL;
    }
    
    Arena *arena = (Arena*)Arena_Malloc(sizeof(Arena));

    if(!arena) {
        return NULL;
    }

    arena->data      = (char*)Arena_Malloc(sizeof(char) * size);
    arena->allocated = 0;
    arena->capacity  = size;

    if(!arena->data) {
        Arena_Free(arena);
        return NULL;
    }

    return arena;
}

inline static void *arena_alloc(Arena *arena, u64 size) {
    if(arena->capacity < arena->allocated + size) {
        u16 done = arena_realloc(arena, arena->capacity + size * 2);

        assert(done == 0);
    }

    void *data = (arena->data + arena->allocated);
    arena->allocated += size;

    return data;
}

inline static void *arena_alloc_aligned(Arena *arena, u64 size, u16 align) {
    u64 shift = ((u64)arena->data + arena->allocated) % align;
    if(shift != 0) shift = align - shift;

    if(arena->capacity < arena->allocated + size + shift) {
        u16 done = arena_realloc(arena, arena->capacity + size * 2);

        assert(done == 0);
    }

    void *data = (arena->data + (arena->allocated + shift));
    arena->allocated += size + shift;

    return data;
}

inline static void arena_flush(Arena *arena) {
    arena->allocated = 0;
}

inline static void arena_free(Arena *arena) {
    Arena_Free(arena->data);
    Arena_Free(arena);
}

inline static u16 arena_realloc(Arena *arena, u64 new_size) {
    void *data = Arena_Realloc(arena->data, new_size);

    if(!data) {
        return 1;
    }

    arena->data     = (char*)data;
    arena->capacity = new_size;

    return 0;
}

#endif // ARENA_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // ARENA_H