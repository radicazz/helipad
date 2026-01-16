/**
 * @file window.hxx
 * @brief Window management utilities using SDL3.
 */

#pragma once

#include <string>
#include <laya/windows/window.hpp>
#include <glm/glm.hpp>

namespace engine {
    /**
     * @brief Types of supported game windows.
     */
    enum class game_window_type { resizable, non_resizable, borderless, fullscreen };

    class game_window {
    public:
        explicit game_window(std::string_view title, const glm::ivec2& size, game_window_type type);
        ~game_window() = default;

        // Resource management - disable copy, enable move
        game_window(const game_window&) = delete;
        game_window& operator=(const game_window&) = delete;
        game_window(game_window&& other) noexcept = default;
        game_window& operator=(game_window&& other) noexcept = default;

        [[nodiscard]] SDL_Window* get_sdl_window() const;
        [[nodiscard]] laya::window& get_laya_window() noexcept;
        [[nodiscard]] const laya::window& get_laya_window() const noexcept;

        [[nodiscard]] std::string get_title() const;
        void set_title(std::string_view new_title);

        [[nodiscard]] glm::ivec2 get_logical_size() const;
        void set_logical_size(const glm::ivec2& size);

        [[nodiscard]] glm::ivec2 get_pixel_size() const;

        /**
         * @brief Set a custom window icon from a file path.
         * @note Do not include a file extension; the engine will try multiple common formats.
         */
        void set_icon(std::string_view icon_path);

    private:
        laya::window m_window;
        std::string m_title;
    };

    inline SDL_Window* game_window::get_sdl_window() const {
        return m_window.native_handle();
    }

    inline laya::window& game_window::get_laya_window() noexcept {
        return m_window;
    }

    inline const laya::window& game_window::get_laya_window() const noexcept {
        return m_window;
    }
}  // namespace engine
