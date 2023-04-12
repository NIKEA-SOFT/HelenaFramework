#ifndef HELENA_TYPES_BENCHMARKSCOPED_HPP
#define HELENA_TYPES_BENCHMARKSCOPED_HPP

#include <Helena/Engine/Log.hpp>

#include <cstdint>
#include <chrono>

namespace Helena::Types
{
    class BenchmarkScoped
    {
        using Timer = std::chrono::steady_clock;

    public:
        BenchmarkScoped(const SourceLocation& location = SourceLocation::Create()) : m_Location{location}, m_Time{Timer::now()} {}
        ~BenchmarkScoped() {
            const std::chrono::duration<float> timeleft = Timer::now() - m_Time;
            const auto formater = Log::Formater<Log::Benchmark>{"{}] Timeleft: {:.6f} sec", m_Location};
            Log::Console<Log::Benchmark>(formater, m_Location.GetFunction(), timeleft.count());
        }
        BenchmarkScoped(const BenchmarkScoped&) = delete;
        BenchmarkScoped(BenchmarkScoped&&) noexcept = delete;
        BenchmarkScoped& operator=(const BenchmarkScoped&) = delete;
        BenchmarkScoped& operator=(BenchmarkScoped&&) noexcept = delete;

    private:
        SourceLocation m_Location;
        Timer::time_point m_Time;
    };
}

#endif // HELENA_TYPES_BENCHMARKSCOPED_HPP