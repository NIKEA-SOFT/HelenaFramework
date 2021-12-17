#ifndef HELENA_TYPES_BENCHMARKSCOPED_HPP
#define HELENA_TYPES_BENCHMARKSCOPED_HPP

#include <Helena/Engine/Log.hpp>

#include <cstdint>
#include <chrono>

namespace Helena::Types
{
    class BenchmarkScoped
    {
        using Timer     = std::chrono::steady_clock;
        using Logger    = Types::BasicLogger;

        struct Benchmark {
            [[nodiscard]] static consteval auto GetPrefix() noexcept {
                return Logger::CreatePrefix("[Benchmark:");
            }

            [[nodiscard]] static consteval auto GetStyle() noexcept {
                return Logger::CreateStyle(Logger::Color::BrightMagenta);
            }
        };

    public:
        BenchmarkScoped(const Types::SourceLocation& location = Types::SourceLocation::Create()) : m_Location{location}, m_Time{Timer::now()} {}
        ~BenchmarkScoped() {
            const std::chrono::duration<float> timeleft = Timer::now() - m_Time;
            Log::Console<Benchmark>("{}] Timeleft: {:.6f} sec", m_Location.GetFunction(), timeleft.count());
        }
        BenchmarkScoped(const BenchmarkScoped&) = delete;
        BenchmarkScoped(BenchmarkScoped&&) noexcept = delete;
        BenchmarkScoped& operator=(const BenchmarkScoped&) = delete;
        BenchmarkScoped& operator=(BenchmarkScoped&&) noexcept = delete;
    private:
        Types::SourceLocation m_Location;
        Timer::time_point m_Time;
    };
}

#endif // HELENA_TYPES_BENCHMARKSCOPED_HPP