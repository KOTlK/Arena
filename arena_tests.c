/*
    This tests are generated by DeepSeek (https://deepseek.com)
    But I modified the code a little so it can ... compile and run
*/

#include <stdio.h>
#include <string.h>
#define ARENA_IMPLEMENTATION
#include "arena.h"


// Test utilities
#define TEST(name) printf("Running test: %s... ", name)
#define PASS() printf("PASS\n")
#define FAIL(reason) printf("FAIL: %s\n", reason); return 1
#define ASSERT(cond, reason) if (!(cond)) { FAIL(reason); }

int test_basic_allocation() {
    TEST("Basic allocation");

    Arena *arena = make_arena(1024);
    ASSERT(arena != NULL, "Arena creation failed");
    ASSERT(arena->capacity == 1024, "Incorrect initial capacity");
    ASSERT(arena->allocated == 0, "Arena should start empty");

    // Allocate some memory
    int *nums = push_array(arena, int, 10);
    ASSERT(nums != NULL, "Allocation failed");
    ASSERT(arena->allocated == 10 * sizeof(int), "Incorrect allocated size");

    // Use the memory
    for (int i = 0; i < 10; i++) {
        nums[i] = i;
    }

    // Allocate more
    char *str = push_array(arena, char, 100);
    ASSERT(str != NULL, "Second allocation failed");
    ASSERT(arena->allocated == (10 * sizeof(int) + 100), "Incorrect allocated size after second allocation");

    strcpy(str, "Arena allocator test");
    ASSERT(strcmp(str, "Arena allocator test") == 0, "String not stored correctly");

    arena_free(arena);
    PASS();
    return 0;
}

int test_reallocation() {
    TEST("Reallocation");

    Arena *arena = make_arena(16); // Small initial size to force realloc
    ASSERT(arena != NULL, "Arena creation failed");

    // This should trigger reallocation
    int *nums = push_array(arena, int, 10);
    ASSERT(nums != NULL, "Allocation failed");
    ASSERT(arena->capacity >= 10 * sizeof(int), "Reallocation didn't increase capacity");

    // Verify the data is intact after reallocation
    for (int i = 0; i < 10; i++) {
        nums[i] = i;
    }
    for (int i = 0; i < 10; i++) {
        ASSERT(nums[i] == i, "Data corrupted after reallocation");
    }

    arena_free(arena);
    PASS();
    return 0;
}

int test_aligned_allocation() {
    TEST("Aligned allocation");

    Arena *arena = make_arena(1024);
    ASSERT(arena != NULL, "Arena creation failed");

    // First allocate something to create potential misalignment
    char *ch = push_array(arena, char, 1);
    ASSERT(ch != NULL, "First allocation failed");

    // Now request aligned memory
    double *dbl = push_struct_aligned(arena, double, 8);
    ASSERT(dbl != NULL, "Aligned allocation failed");

    // Check alignment
    uintptr_t addr = (uintptr_t)dbl;
    ASSERT(addr % 8 == 0, "Memory not properly aligned");

    // Verify we can use the memory
    *dbl = 3.14159;
    ASSERT(*dbl == 3.14159, "Value not stored correctly");

    arena_free(arena);
    PASS();
    return 0;
}

int test_arena_flush() {
    TEST("Arena flush");

    Arena *arena = make_arena(1024);
    ASSERT(arena != NULL, "Arena creation failed");

    // Allocate some memory
    int *nums = push_array(arena, int, 10);
    ASSERT(nums != NULL, "Allocation failed");
    ASSERT(arena->allocated == 10 * sizeof(int), "Incorrect allocated size");

    // Flush the arena
    arena_flush(arena);
    ASSERT(arena->allocated == 0, "Arena not flushed");

    // Allocate again - should reuse the same memory
    int *nums2 = push_array(arena, int, 5);
    ASSERT(nums2 != NULL, "Allocation after flush failed");
    ASSERT(nums2 == nums, "Memory not reused after flush");

    arena_free(arena);
    PASS();
    return 0;
}

int test_edge_cases() {
    TEST("Edge cases");

    // Test zero-size arena
    Arena *arena = make_arena(0);
    ASSERT(arena == NULL, "Zero-size arena should fail");

    // Test allocation failures
    arena = make_arena(16);
    ASSERT(arena != NULL, "Arena creation failed");

    // Try to allocate more than initial size (should realloc)
    int *big = push_array(arena, int, 100);
    ASSERT(big != NULL, "Large allocation failed");
    ASSERT(arena->capacity >= 100 * sizeof(int), "Reallocation didn't happen");

    arena_free(arena);
    PASS();
    return 0;
}

int test_custom_allocators() {
    TEST("Custom allocators");

    // Define custom allocators
    #define ARENA_CUSTOM_ALLOC
    #define Arena_Malloc(size) malloc(size)
    #define Arena_Free(data) free(data)
    #define Arena_Realloc(data, size) realloc(data, size)

    // Include arena.h again to use custom allocators
    #include "arena.h"

    Arena *arena = make_arena(1024);
    ASSERT(arena != NULL, "Arena creation with custom allocators failed");

    int *nums = push_array(arena, int, 10);
    ASSERT(nums != NULL, "Allocation with custom allocators failed");

    arena_free(arena);

    // Undefine to avoid affecting other tests
    #undef ARENA_CUSTOM_ALLOC
    #undef Arena_Malloc
    #undef Arena_Free
    #undef Arena_Realloc

    #include "arena.h"

    PASS();
    return 0;
}

int main() {
    printf("Starting arena allocator tests...\n\n");

    int failures = 0;

    failures += test_basic_allocation();
    failures += test_reallocation();
    failures += test_aligned_allocation();
    failures += test_arena_flush();
    failures += test_edge_cases();
    failures += test_custom_allocators();

    printf("\nTests completed. %d failures.\n", failures);
    return failures;
}