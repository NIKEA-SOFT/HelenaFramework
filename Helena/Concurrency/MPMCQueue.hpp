#ifndef HELENA_CONCURRENCY_MPMCQUEUE_HPP
#define HELENA_CONCURRENCY_MPMCQUEUE_HPP

#include <Helena/Defines.hpp>
#include <Helena/Concurrency/Internal.hpp>

#include <atomic>
#include <memory>
#include <optional>
#include <utility>


namespace Helena::Concurrency {

    // TODO
    template <typename T, std::uint32_t Size>
    class MPMCQueue HF_FINAL {

        using storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

    public:
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = std::uint32_t;

    public:
        MPMCQueue() = default;
        ~MPMCQueue() = default;
        MPMCQueue(const MPMCQueue&) = delete;
        MPMCQueue& operator=(const MPMCQueue&) = delete;
        MPMCQueue(MPMCQueue&&) noexcept = delete;
        MPMCQueue& operator=(MPMCQueue&&) noexcept = delete;

    private:

    };
}

#include <Helena/Concurrency/MPMCQueue.ipp>

#endif // HELENA_CONCURRENCY_MPMCQUEUE_HPP
