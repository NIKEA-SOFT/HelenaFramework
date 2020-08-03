#ifndef COMMON_HFSINGLETON_HPP
#define COMMON_HFSINGLETON_HPP

#include <memory>
#include <mutex>
#include <thread>

namespace Helena
{
    template <typename Type>
    class HFSingleton
    {
    public:
        static Type& GetInstance() {
            static Type instance {};
            return instance;
        }

        HFSingleton() = default;
        ~HFSingleton() = default;
        HFSingleton(const HFSingleton&) = delete;
        HFSingleton(HFSingleton&&) = delete;
        HFSingleton& operator=(const HFSingleton&) = delete;
        HFSingleton& operator=(HFSingleton&&) = delete;
    };
}

#endif