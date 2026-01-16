/*
 * @file safety.hxx
 * @brief Custom error handling and safety utilities.
 */

#pragma once

#include <type_traits>
#include <stdexcept>
#include <format>
#include <source_location>
#include <functional>
#include <optional>
#include <utility>

#include "config.hxx"

namespace engine {
    /**
     * @brief A std::format compatible version of std::runtime_error.
     */
    class error_message : public std::runtime_error {
    public:
        template <class... Args>
        error_message(std::string_view fmt, Args... args)
            : std::runtime_error(std::vformat(fmt, std::make_format_args(args...))) {
        }
    };

    bool message_box_info(std::string_view title, std::string_view message);
    bool message_box_error(std::string_view title, std::string_view message);

    /**
     * @brief Throw an exception if a condition is false.
     * @param condition Boolean condition to test.
     * @param msg A description of the error.
     * @param loc Source location where the check is performed.
     * @note This function only performs the check if `is_paranoid_build` is true.
     */
    inline void paranoid_ensure([[maybe_unused]] const bool condition,
                                [[maybe_unused]] std::string_view msg,
                                [[maybe_unused]] const std::source_location
                                    loc = std::source_location::current()) {
        if constexpr (is_paranoid_build) {
            if (!condition) {
                throw error_message("{}:{} [{}] -> {}", loc.file_name(), loc.line(),
                                    loc.function_name(), msg);
            }
        }
    }

    /**
     * @brief Invoke a callable and wrap the result in an optional incase something goes wrong.
     * @tparam F Type of the callable.
     * @tparam Args Types of the arguments to pass to the callable.
     * @param fn The callable to invoke.
     * @param args Arguments to pass to the callable.
     * @return An `std::optional` either containing the result of the callable, or not.
     */
    template <class F, class... Args>
        requires std::invocable<F, Args...>
    [[nodiscard]]
    auto invoke_optional(F&& function,
                         Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>)
        -> std::optional<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;
        static_assert(!std::is_void_v<return_type>,
                      "invoke_optional should only be used with callables that return a value.");
        using function_type = std::remove_cvref_t<F>;
        using optional_type = std::optional<return_type>;

        if constexpr (std::is_pointer_v<function_type> || std::is_member_pointer_v<function_type>) {
            if (function == nullptr) {
                return optional_type{};
            }
        } else if constexpr (requires(const function_type& candidate) {
                                 static_cast<bool>(candidate);
                             }) {
            if (!function) {
                return optional_type{};
            }
        }

        return optional_type{std::invoke(std::forward<F>(function), std::forward<Args>(args)...)};
    }

    /**
     * @brief Invoke a callable that returns void, ignoring empty callables.
     * @tparam F Type of the callable.
     * @tparam Args Types of the arguments to pass to the callable.
     * @param function The callable to invoke.
     * @param args Arguments to pass to the callable.
     */
    template <class F, class... Args>
        requires std::invocable<F, Args...>
    void invoke_void(F&& function,
                     Args&&... args) noexcept(std::is_nothrow_invocable_v<F, Args...>) {
        using return_type = std::invoke_result_t<F, Args...>;
        static_assert(std::is_void_v<return_type>,
                      "invoke_void requires a callable that returns nothing");
        using function_type = std::remove_cvref_t<F>;

        if constexpr (std::is_pointer_v<function_type> || std::is_member_pointer_v<function_type>) {
            if (function == nullptr) {
                return;
            }
        } else if constexpr (requires(const function_type& candidate) {
                                 static_cast<bool>(candidate);
                             }) {
            if (!function) {
                return;
            }
        }

        std::invoke(std::forward<F>(function), std::forward<Args>(args)...);
    }
}  // namespace engine
