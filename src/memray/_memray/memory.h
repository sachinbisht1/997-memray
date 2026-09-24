#pragma once

#include <cstddef>
#include <new>
#include <string>

namespace memray::tracking_api {

void* mmapAllocate(std::size_t bytes);
void mmapDeallocate(void* ptr, std::size_t bytes) noexcept;

template<typename T>
class MmapAllocator
{
  public:
    using value_type = T;

    MmapAllocator() noexcept = default;

    template<typename U>
    MmapAllocator(const MmapAllocator<U>&) noexcept
    {
    }

    [[nodiscard]] T* allocate(std::size_t count);

    void deallocate(T* ptr, std::size_t count) noexcept;

    template<typename U>
    bool operator==(const MmapAllocator<U>&) const noexcept
    {
        return true;
    }

    template<typename U>
    bool operator!=(const MmapAllocator<U>&) const noexcept
    {
        return false;
    }
};

template<typename T>
T*
MmapAllocator<T>::allocate(std::size_t count)
{
    if (count > static_cast<std::size_t>(-1) / sizeof(T)) {
        throw std::bad_alloc{};
    }

    const std::size_t bytes = count * sizeof(T);

    if (bytes == 0) {
        return nullptr;
    }

    return static_cast<T*>(mmapAllocate(bytes));
}

template<typename T>
void
MmapAllocator<T>::deallocate(T* ptr, std::size_t count) noexcept
{
    if (!ptr || count == 0) {
        return;
    }

    mmapDeallocate(ptr, count * sizeof(T));
}

using MmapString = std::basic_string<char, std::char_traits<char>, MmapAllocator<char>>;

}  // namespace memray::tracking_api
