#ifndef HELENA_TYPES_RWLOCK_HPP
#define HELENA_TYPES_RWLOCK_HPP

#include <Helena/Platform/Defines.hpp>

#include <atomic>
#include <limits>

namespace Helena::Types
{
    class RWLock final
    {
        using RWAtomic = std::atomic<std::size_t>;
        using Value = typename RWAtomic::value_type;

        static constexpr Value MaxSize = (std::numeric_limits<Value>::max)();
        static constexpr Value MaxBits = std::numeric_limits<Value>::digits;
        static constexpr Value HalfBits = MaxBits / 2;

        enum class EGuard : bool {
            Writer,
            Reader
        };

        template <EGuard Type>
        class RWGuard {
        public:
            RWGuard(RWAtomic& rwCounter) noexcept : m_RWCounter{rwCounter} { Lock(); }
            ~RWGuard() noexcept { Unlock(); }
            RWGuard(const RWGuard&) = delete;
            RWGuard(RWGuard&&) noexcept = delete;
            RWGuard& operator=(const RWGuard&) = delete;
            RWGuard& operator=(RWGuard&&) noexcept = delete;

        private:
            void Lock() const noexcept
            {
                if constexpr(Type == EGuard::Writer) {
                    auto rwCounter = m_RWCounter.load(std::memory_order_relaxed);
                    while(!m_RWCounter.compare_exchange_weak(rwCounter, WritersInc(rwCounter),
                        std::memory_order_release, std::memory_order_relaxed));
                    while(true) {
                        rwCounter = m_RWCounter.load(std::memory_order_relaxed);
                        while(Writable(rwCounter)) {
                            if(m_RWCounter.compare_exchange_strong(rwCounter, WritingInc(rwCounter),
                                std::memory_order_release, std::memory_order_relaxed)) {
                                return;
                            }
                        } HELENA_PROCESSOR_YIELD();
                    }
                } else while(true) {
                    auto rwCounter = m_RWCounter.load(std::memory_order_relaxed);
                    while(Readable(rwCounter)) {
                        if(m_RWCounter.compare_exchange_strong(rwCounter, ReadersInc(rwCounter),
                            std::memory_order_release, std::memory_order_relaxed)) {
                            return;
                        }
                    } HELENA_PROCESSOR_YIELD();
                }
            }

            void Unlock() const noexcept
            {
                auto rwCounter = m_RWCounter.load(std::memory_order_relaxed);
                if constexpr(Type == EGuard::Writer) {
                    while(!m_RWCounter.compare_exchange_weak(rwCounter, WritersDec(rwCounter),
                        std::memory_order_release, std::memory_order_relaxed));
                } else {
                    while(!m_RWCounter.compare_exchange_weak(rwCounter, ReadersDec(rwCounter),
                        std::memory_order_release, std::memory_order_relaxed));
                }
            }

            [[nodiscard]] static Value WritingInc(Value rwCounter) noexcept {
                return SetBitField(rwCounter, MaxBits - 1, 1, 1);
            }

            [[nodiscard]] static Value WritersInc(Value rwCounter) noexcept {
                return SetBitField(rwCounter, HalfBits, HalfBits, GetBitField(rwCounter, HalfBits, HalfBits) + 1);
            }

            [[nodiscard]] static Value WritersDec(Value rwCounter) noexcept {
                const auto writers = GetBitField(rwCounter, HalfBits, HalfBits) - 1;
                return SetBitField(rwCounter, HalfBits, HalfBits, SetBitField(writers, HalfBits - 1, 1, 0));
            }

            [[nodiscard]] static Value Writers(Value rwCounter) noexcept {
                return GetBitField(rwCounter, HalfBits, HalfBits);
            }

            [[nodiscard]] static bool Writable(Value rwCounter) noexcept {
                return !Readers(rwCounter) && !GetBitField(rwCounter, MaxBits - 1, 1);
            }

            [[nodiscard]] static Value ReadersInc(Value rwCounter) noexcept {
                return SetBitField(rwCounter, 0, HalfBits, GetBitField(rwCounter, 0, HalfBits) + 1);
            }

            [[nodiscard]] static Value ReadersDec(Value rwCounter) noexcept {
                return SetBitField(rwCounter, 0, HalfBits, GetBitField(rwCounter, 0, HalfBits) - 1);
            }

            [[nodiscard]] static Value Readers(Value rwCounter) noexcept {
                return GetBitField(rwCounter, 0, HalfBits);
            }

            [[nodiscard]] static bool Readable(Value rwCounter) noexcept {
                return !Writers(rwCounter);
            }

            [[nodiscard]] static Value GetBitField(Value value, Value start, Value length) noexcept {
                return (value >> start) & ((Value{1} << length) - 1);
            }

            [[nodiscard]] static Value SetBitField(Value value, Value start, Value length, Value newValue) noexcept {
                const auto mask = (Value{1} << length) - 1;
                return (value & ~(mask << start)) | ((newValue & mask) << start);
            }

        private:
            RWAtomic& m_RWCounter;
        };

    public:
        RWLock() noexcept = default;
        ~RWLock() noexcept = default;
        RWLock(const RWLock&) = delete;
        RWLock(RWLock&&) noexcept = delete;
        RWLock& operator=(const RWLock&) = delete;
        RWLock& operator=(RWLock&&) noexcept = delete;

        [[nodiscard]] RWGuard<EGuard::Writer> CreateWriter() noexcept {
            return m_RWCounter;
        }

        [[nodiscard]] RWGuard<EGuard::Reader> CreateReader() noexcept {
            return m_RWCounter;
        }

    private:
        RWAtomic m_RWCounter {};
    };
}
#endif // HELENA_TYPES_RWLOCK_HPP