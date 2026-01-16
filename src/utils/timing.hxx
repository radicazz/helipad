/**
 * @file timing.hxx
 * @brief Timing related functions and utilities.
 */

#pragma once

#include <cstdint>

namespace engine {
    /**
     * @brief Get the current high resolution performance counter value.
     * @return Value since the engine started.
     * @note This value is only meaningful when compared to another value from this function.
     */
    [[nodiscard]] std::uint64_t performance_counter_value_current() noexcept;

    /**
     * @brief Get the time between two performance counter values.
     * @param start_value The starting performance counter value.
     * @param end_value The ending performance counter value.
     * @return Time in seconds between the two counter values.
     */
    [[nodiscard]] float performance_counter_seconds_between(std::uint64_t start_value,
                                                            std::uint64_t end_value) noexcept;

    /**
     * @brief Get the time since a previous performance counter value.
     * @param start_value The performance counter value to measure from.
     * @return Time in seconds since the provided counter value.
     *
     * @code
     * const std::uint64_t start = performance_counter_now();
     * // ... do some work ...
     * float elapsed_seconds = performance_counter_seconds_since(start);
     * std::cout << "Elapsed time: " << elapsed_seconds << " seconds\n";
     * @endcode
     */
    [[nodiscard]] inline float performance_counter_seconds_since(
        std::uint64_t start_value) noexcept {
        const std::uint64_t now = performance_counter_value_current();
        return performance_counter_seconds_between(start_value, now);
    }

    constexpr float ticks_rate_to_interval(const float ticks_per_second) noexcept {
        return 1.f / ticks_per_second;
    }

    constexpr float ticks_interval_to_rate(const float tick_interval_seconds) noexcept {
        return 1.f / tick_interval_seconds;
    }
}  // namespace engine
