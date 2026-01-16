/**
 * @file viewport.hxx
 * @brief Viewport definitions header.
 */

#pragma once

#include <string>

#include <glm/glm.hpp>

struct SDL_Renderer;

namespace engine {
    class game_camera;
    class game_renderer;

    /**
     * @brief A rectangular render target in window space using normalized coordinates.
     */
    class game_viewport {
    public:
        game_viewport() = delete;
        game_viewport(std::string_view name, const glm::vec2& position_normalized,
                      const glm::vec2& size_normalized);
        ~game_viewport() = default;

        game_viewport(const game_viewport&) = default;
        game_viewport& operator=(const game_viewport&) = default;
        game_viewport(game_viewport&&) = default;
        game_viewport& operator=(game_viewport&&) = default;

        [[nodiscard]] std::string_view get_name() const;

        void set_position(const glm::vec2& normalized_position);
        void set_size(const glm::vec2& normalized_size);
        void set_rect(const glm::vec2& normalized_position, const glm::vec2& normalized_size);

        [[nodiscard]] glm::vec2 get_position() const;
        [[nodiscard]] glm::vec2 get_size() const;

        [[nodiscard]] glm::vec2 get_position_pixels() const;
        [[nodiscard]] glm::vec2 get_size_pixels() const;

        /** Compute and apply viewport to SDL, given the renderer output size. */
        void apply_to_sdl(game_renderer& renderer) const;

        // Transform helpers using the camera (use cached pixel values).
        glm::mat3 get_view_matrix(const game_camera& camera) const;
        glm::vec2 world_to_screen(const game_camera& camera, const glm::vec2& world_pos) const;
        glm::vec2 screen_to_world(const game_camera& camera, const glm::vec2& screen_pos) const;

        std::tuple<glm::vec2, glm::vec2> get_visible_area_world(const game_camera& camera) const;
        bool is_in_view(const game_camera& camera, const glm::vec2& position,
                        const glm::vec2& size) const;

        void clamp_camera_to_bounds(game_camera& camera) const;

    public:
        static constexpr std::string_view default_name = "main";

    private:
        [[nodiscard]] static glm::vec2 clamp_normalized(const glm::vec2& vec);

    private:
        std::string m_name;

        glm::vec2 m_position;  ///< Viewport position normalized (0.f to 1.f) on x & y.
        glm::vec2 m_size;      ///< Viewport size normalized  (0.f to 1.f) on x & y.

        mutable glm::vec2 m_cached_position_pixels;
        mutable glm::vec2 m_cached_size_pixels;
    };

    inline std::string_view game_viewport::get_name() const {
        return m_name;
    }

    inline void game_viewport::set_position(const glm::vec2& new_position) {
        m_position = clamp_normalized(new_position);
    }

    inline void game_viewport::set_size(const glm::vec2& new_size) {
        m_size = clamp_normalized(new_size);
    }

    inline glm::vec2 game_viewport::get_position() const {
        return m_position;
    }

    inline glm::vec2 game_viewport::get_size() const {
        return m_size;
    }

    inline glm::vec2 game_viewport::get_position_pixels() const {
        return m_cached_position_pixels;
    }

    inline glm::vec2 game_viewport::get_size_pixels() const {
        return m_cached_size_pixels;
    }

    inline glm::vec2 game_viewport::clamp_normalized(const glm::vec2& vec) {
        return glm::clamp(vec, glm::vec2{0.f}, glm::vec2{1.f});
    }
}  // namespace engine
