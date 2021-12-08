#ifndef HELENA_TYPES_BENCHMARKSCOPED_HPP
#define HELENA_TYPES_BENCHMARKSCOPED_HPP

#include <Helena/Engine/Log.hpp>
#include <Helena/Types/SourceLocation.hpp>

#include <cstdint>
#include <chrono>

namespace Helena::Types 
{
	class BenchmarkScoped
	{
		using Timer = std::chrono::steady_clock;

		struct Benchmark {
			[[nodiscard]] static consteval auto GetPrefix() noexcept {
				return Log::CreatePrefix("[Benchmark:");
			}

			[[nodiscard]] static consteval auto GetStyle() noexcept {
				return Log::CreateStyle(Log::Color::BrightMagenta);
			}
		};

	public:
		BenchmarkScoped(const Types::SourceLocation& location = Types::SourceLocation::Create()) 
			: m_Location{location}
			, m_Time{Timer::now()} {}
		~BenchmarkScoped() {
			const std::chrono::duration<float> timeleft = Timer::now() - m_Time;
			HELENA_MSG(Benchmark, "{}] Timeleft: {:.6f}", m_Location.GetFunction(), timeleft.count());
		}
		BenchmarkScoped(const BenchmarkScoped&) = default;
		BenchmarkScoped(BenchmarkScoped&&) noexcept = default;
		BenchmarkScoped& operator=(const BenchmarkScoped&) = default;
		BenchmarkScoped& operator=(BenchmarkScoped&&) noexcept = default;
	private:
		const Types::SourceLocation m_Location;
		const Timer::time_point m_Time;
	};
}

#endif // HELENA_TYPES_BENCHMARKSCOPED_HPP