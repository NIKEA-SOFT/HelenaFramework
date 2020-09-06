#ifndef COMMON_SINGLETON_HPP
#define COMMON_SINGLETON_HPP

namespace Helena
{
    template <typename Type>
    class Singleton
    {
    public:
        static Type& GetInstance() {
            static Type instance {};
            return instance;
        }

        Singleton() = default;
        ~Singleton() = default;
        Singleton(const HFSingleton&) = delete;
        Singleton(HFSingleton&&) = delete;
        Singleton& operator=(const HFSingleton&) = delete;
        Singleton& operator=(HFSingleton&&) = delete;
    };
}

#endif // COMMON_SINGLETON_HPP