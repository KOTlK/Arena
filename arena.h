/*
    Create implementation: #define ARENA_IMPLEMENTATION before including this file
*/

#ifndef ARENA_H
#define ARENA_H

#ifndef REGION_MIN_SIZE
#define REGION_MIN_SIZE 65536
#endif

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

#include <stdlib.h>

typedef struct Region Region;

struct Region {
    char   *data;
    Region *next;
    u64     capacity;
    u64     allocated;
};

typedef struct Arena {
    Region *root;
    u64     total;
} Arena;

#define push_array(arena, type, count) (type*)(arena_alloc(arena, sizeof(type) * count))
#define push_struct(arena, type) push_array(arena, type, 1)
#define push_array_aligned(arena, type, count, align) (type*)(arena_alloc_aligned(arena, sizeof(type) * count, align))
#define push_struct_aligned(arena, type, align) (type*)(arena_alloc_aligned(arena, sizeof(type), align))

static Region *make_region(u64 capacity);
static void    region_flush(Region *region);
static void    region_free(Region *region);

inline static Region *region_get_fit(Region *start, u64 size);
inline static Region *region_get_fit_aligned(Region *start, u64 size, u16 align);

static        Arena *make_arena(u64 size);
inline static void  *arena_alloc(Arena *arena, u64 size);
inline static void  *arena_alloc_aligned(Arena *arena, u64 size, u16 align);
inline static void   arena_flush(Arena *arena);
inline static void   arena_free(Arena *arena);

#ifdef ARENA_IMPLEMENTATION

static Region *make_region(u64 capacity) {
    if (capacity < REGION_MIN_SIZE) {
        capacity = REGION_MIN_SIZE;
    }

    char *data = (char*)malloc(sizeof(Region) + capacity);

    Region *region = (Region*)data;

    if(!region) {
        return NULL;
    }

    region->data        = (char*)(data + sizeof(Region));
    region->capacity    = capacity;
    region->allocated   = 0;
    region->next        = NULL;

    return region;
}

static void region_flush(Region *region) {
    region->allocated = 0;

    if(region->next) {
        region_flush(region->next);
    }
}

static void region_free(Region *region) {
    if(region->next) {
        region_free(region->next);
    }

    free(region);
}

inline static Region *region_get_fit(Region *start, u64 size) {
    if(start->capacity > start->allocated + size) {
        return start;
    }

    while(start->next != NULL) {
        start = start->next;

        if(start->capacity > start->allocated + size) {
            return start;
        }
    }

    start->next = make_region(size << 1);

    return start->next;
}

inline static Region *region_get_fit_aligned(Region *start, u64 size, u16 align) {
    u64 shift = ((u64)start->data + start->allocated) % align;
    if(shift != 0) shift = align - shift;

    if(start->capacity > start->allocated + shift + size) {
        return start;
    }

    while(start->next != NULL) {
        start = start->next;
        shift = ((u64)start->data + start->allocated) % align;
        if(shift != 0) shift = align - shift;

        if(start->capacity > start->allocated + shift + size) {
            return start;
        }
    }

    start->next = make_region(size << 1);

    return start->next;
}

static Arena *make_arena(u64 size) {
    Arena *arena = (Arena*)malloc(sizeof(Arena));

    if(!arena) {
        return NULL;
    }

    arena->root  = make_region(size);
    arena->total = 0;

    if(!arena->root) {
        free(arena);
        return NULL;
    }

    return arena;
}

inline static void *arena_alloc(Arena *arena, u64 size) {
    Region *region = region_get_fit(arena->root, size);

    void *data = (region->data + region->allocated);
    region->allocated += size;
    arena->total += size;
    return data;
}

inline static void *arena_alloc_aligned(Arena *arena, u64 size, u16 align) {
    Region *region = region_get_fit_aligned(arena->root, size, align);
    u64 shift = ((u64)region->data + region->allocated) % align;
    if(shift != 0) shift = align - shift;

    void *data = (region->data + (region->allocated + shift));
    region->allocated += size + shift;

    arena->total += size + shift;

    return data;
}

inline static void arena_flush(Arena *arena) {
    region_flush(arena->root);
    arena->total = 0;
}

inline static void arena_free(Arena *arena) {
    region_free(arena->root);
    free(arena);
}

#endif // ARENA_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // ARENA_H