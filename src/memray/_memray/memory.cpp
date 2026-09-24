#include "memory.h"

#ifdef __linux__

#include <sys/mman.h>
#include <unistd.h>

#include "tracking_api.h"

namespace memray::tracking_api {

namespace {

std::size_t
pageSize()
{
    const long value = sysconf(_SC_PAGESIZE);
    if (value <= 0) {
        throw std::bad_alloc{};
    }

    return static_cast<std::size_t>(value);
}

bool
roundUpToPage(std::size_t bytes, std::size_t page, std::size_t& result)
{
    if (bytes > static_cast<std::size_t>(-1) - (page - 1)) {
        return false;
    }

    result = ((bytes + page - 1) / page) * page;
    return true;
}

}  // namespace

void*
mmapAllocate(std::size_t bytes)
{
    const std::size_t page = pageSize();

    std::size_t length;
    if (!roundUpToPage(bytes, page, length)) {
        throw std::bad_alloc{};
    }

    RecursionGuard guard;

    void* result = ::mmap(
            nullptr,
            length,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0);

    if (result == MAP_FAILED) {
        throw std::bad_alloc{};
    }

    return result;
}

void
mmapDeallocate(void* ptr, std::size_t bytes) noexcept
{
    if (!ptr || bytes == 0) {
        return;
    }

    const long value = sysconf(_SC_PAGESIZE);
    if (value <= 0) {
        return;
    }

    const std::size_t page = static_cast<std::size_t>(value);

    std::size_t length;
    if (!roundUpToPage(bytes, page, length)) {
        return;
    }

    RecursionGuard guard;
    ::munmap(ptr, length);
}

}  // namespace memray::tracking_api

#else

namespace memray::tracking_api {

void*
mmapAllocate(std::size_t)
{
    throw std::bad_alloc{};
}

void
mmapDeallocate(void*, std::size_t) noexcept
{
}

}  // namespace memray::tracking_api

#endif
