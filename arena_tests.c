#define ARENA_IMPLEMENTATION
#include "arena.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_basic_allocation() {
    printf("Running test_basic_allocation...\n");

    Arena *arena = make_arena(1024);
    assert(arena != NULL);
    assert(arena->root != NULL);
    assert(arena->total == 0);

    // Allocate some integers
    int *a = push_struct(arena, int);
    *a = 42;
    assert(*a == 42);
    assert(arena->total == sizeof(int));

    int *b = push_struct(arena, int);
    *b = 100;
    assert(*b == 100);
    assert(arena->total == 2 * sizeof(int));

    // Allocate an array
    double *arr = push_array(arena, double, 10);
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 1.5;
    }
    assert(arr[5] == 7.5);
    assert(arena->total == (2 * sizeof(int) + 10 * sizeof(double)));

    arena_free(arena);
    printf("Passed!\n\n");
}

void test_region_creation() {
    printf("Running test_region_creation...\n");

    // Test with size smaller than REGION_MIN_SIZE
    Arena *arena = make_arena(100);
    assert(arena != NULL);
    assert(arena->root != NULL);
    assert(arena->root->capacity == REGION_MIN_SIZE);

    // Test with larger size
    u64 large_size = REGION_MIN_SIZE * 2;
    Arena *large_arena = make_arena(large_size);
    assert(large_arena != NULL);
    assert(large_arena->root != NULL);
    assert(large_arena->root->capacity >= large_size);

    arena_free(arena);
    arena_free(large_arena);
    printf("Passed!\n\n");
}

void test_multiple_regions() {
    printf("Running test_multiple_regions...\n");

    // Create arena with small initial region
    Arena *arena = make_arena(128);
    assert(arena != NULL);

    // Allocate enough to trigger new region creation
    char *big = push_array(arena, char, REGION_MIN_SIZE);
    assert(big != NULL);
    assert(arena->root->next != NULL); // Should have created a new region

    // Fill the big array
    memset(big, 'A', REGION_MIN_SIZE);
    assert(big[0] == 'A');
    assert(big[REGION_MIN_SIZE - 1] == 'A');

    arena_free(arena);
    printf("Passed!\n\n");
}

void test_aligned_allocation() {
    printf("Running test_aligned_allocation...\n");

    Arena *arena = make_arena(1024);
    assert(arena != NULL);

    // First allocate something to potentially misalign
    char *a = push_struct(arena, char);
    *a = 'x';

    // Now request aligned memory
    uint64_t *aligned = push_struct_aligned(arena, uint64_t, 16);
    assert(((uintptr_t)aligned % 16) == 0);

    *aligned = 0xDEADBEEF;
    assert(*aligned == 0xDEADBEEF);

    // Test array alignment
    double *aligned_arr = push_array_aligned(arena, double, 5, 32);
    assert(((uintptr_t)aligned_arr % 32) == 0);
    for (int i = 0; i < 5; i++) {
        aligned_arr[i] = i * 3.14;
    }
    assert(aligned_arr[3] == 3 * 3.14);

    arena_free(arena);
    printf("Passed!\n\n");
}

void test_flush() {
    printf("Running test_flush...\n");

    Arena *arena = make_arena(1024);
    assert(arena != NULL);

    // Allocate some data
    int *a = push_struct(arena, int);
    *a = 123;
    double *b = push_struct(arena, double);
    *b = 456.789;

    assert(arena->total > 0);
    u64 total_before = arena->total;

    // Flush the arena
    arena_flush(arena);
    assert(arena->total == 0);

    // Allocate again - should reuse the same memory
    int *c = push_struct(arena, int);
    *c = 321;
    assert(c == a); // Should get same pointer back after flush

    // Allocate more than before to test region chain flush
    char *big = push_array(arena, char, 2048);
    memset(big, 'B', 2048);
    arena_flush(arena);
    assert(arena->total == 0);

    arena_free(arena);
    printf("Passed!\n\n");
}

void test_edge_cases() {
    printf("Running test_edge_cases...\n");

    // Test zero-sized arena (should use REGION_MIN_SIZE)
    Arena *arena = make_arena(0);
    assert(arena != NULL);
    assert(arena->root->capacity == REGION_MIN_SIZE);
    arena_free(arena);

    // Test allocation of zero bytes
    arena = make_arena(1024);
    void *ptr = arena_alloc(arena, 0);
    assert(ptr != NULL); // Implementation returns valid pointer
    assert(arena->total == 0);

    // Test very large allocation
    void *large = arena_alloc(arena, REGION_MIN_SIZE * 3);
    assert(large != NULL);
    assert(arena->root->next != NULL); // Should have created new region

    arena_free(arena);
    printf("Passed!\n\n");
}

int main() {
    printf("Starting arena allocator tests...\n\n");

    test_basic_allocation();
    test_region_creation();
    test_multiple_regions();
    test_aligned_allocation();
    test_flush();
    test_edge_cases();

    printf("All tests passed!\n");
    return 0;
}