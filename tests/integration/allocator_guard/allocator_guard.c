#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

static void *(*real_malloc)(size_t);
static void *(*real_calloc)(size_t, size_t);
static void *(*real_realloc)(void *, size_t);
static void (*real_free)(void *);

static int guard_enabled = 0;

__attribute__((constructor))
static void initialize_guard(void)
{
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_calloc = dlsym(RTLD_NEXT, "calloc");
    real_realloc = dlsym(RTLD_NEXT, "realloc");
    real_free = dlsym(RTLD_NEXT, "free");
}

static void check_guard(void)
{
    if (guard_enabled) {
        const char message[] =
                "allocator_guard: process allocator used\n";
        write(STDERR_FILENO, message, sizeof(message) - 1);
        _exit(97);
    }
}

void *malloc(size_t size)
{
    check_guard();
    return real_malloc(size);
}

void *calloc(size_t nmemb, size_t size)
{
    check_guard();
    return real_calloc(nmemb, size);
}

void *realloc(void *ptr, size_t size)
{
    check_guard();
    return real_realloc(ptr, size);
}

void free(void *ptr)
{
    check_guard();
    real_free(ptr);
}

#include <sys/mman.h>

int memray_allocator_guard_mmap(size_t size)
{
    void *ptr;

    guard_enabled = 1;

    ptr = mmap(
            NULL,
            size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0);

    if (ptr == MAP_FAILED) {
        guard_enabled = 0;
        return -1;
    }

    if (munmap(ptr, size) != 0) {
        guard_enabled = 0;
        return -1;
    }

    guard_enabled = 0;
    return 0;
}
