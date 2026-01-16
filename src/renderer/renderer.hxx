#pragma once

#include "sprite.hxx"
#include "text.hxx"

#include <unordered_map>
#include <memory>
#include <string_view>
#include <string>

struct TTF_TextEngine;

namespace engine {
    class game_camera;
    class game_viewport;

    /**
     * @brief Handles rendering of sprites and text with support for camera and viewport.
     */
    class game_renderer {
    public:
        explicit game_renderer(SDL_Window* window);
        ~game_renderer();

        // Resource management - disable copy, enable move
        game_renderer(const game_renderer&) = delete;
        game_renderer& operator=(const game_renderer&) = delete;
        game_renderer(game_renderer&& other) noexcept;
        game_renderer& operator=(game_renderer&& other) noexcept;

        [[nodiscard]] SDL_Renderer* get_sdl_renderer() const;
        [[nodiscard]] TTF_TextEngine* get_sdl_text_engine() const;

        void draw_begin();
        void draw_end();

        void set_camera(const game_camera* camera);
        [[nodiscard]] const game_camera* get_camera() const;

        // --- Legacy single viewport access (points into registry) ---
        void set_viewport(const game_viewport* viewport);
        [[nodiscard]] const game_viewport* get_viewport() const;

        // --- Multi-viewport API ---
        // Create or fetch viewport by name; if creating, specify normalized rect.
        game_viewport& viewport_get_or_create(std::string_view name,
                                              const glm::vec2& pos_norm = {0.f, 0.f},
                                              const glm::vec2& size_norm = {1.f, 1.f});
        game_viewport* viewport_get(std::string_view name);
        bool viewport_remove(std::string_view name);
        [[nodiscard]] const std::unordered_map<std::string, std::unique_ptr<game_viewport>>&
        viewports() const {
            return m_viewports;
        }
        [[nodiscard]] game_viewport* viewport_main();  // convenience "main"

        void sprite_draw_world(const game_sprite* sprite, const glm::vec2& world_position);
        void sprite_draw_screen(const game_sprite* sprite, const glm::vec2& screen_position);

        void text_draw_world(const game_text_dynamic* text, const glm::vec2& world_position);
        void text_draw_screen(const game_text_dynamic* text, const glm::vec2& screen_position);

        void text_draw_screen(const game_text_static* text, const glm::vec2& screen_position);

        /**
         * @brief Get the output size of the renderer.
         * @return glm::vec2 representing the output size in pixels.
         */
        [[nodiscard]] glm::vec2 get_output_size() const;

        // Future: expose iteration rendering hook if needed

    private:
        SDL_Renderer* m_sdl_renderer;
        TTF_TextEngine* m_sdl_text_engine;
        const game_camera* m_camera;
        const game_viewport* m_viewport;
        std::unordered_map<std::string, std::unique_ptr<game_viewport>> m_viewports;
    };

    inline SDL_Renderer* game_renderer::get_sdl_renderer() const {
        return m_sdl_renderer;
    }
    inline TTF_TextEngine* game_renderer::get_sdl_text_engine() const {
        return m_sdl_text_engine;
    }

    inline void game_renderer::set_camera(const game_camera* cam) {
        m_camera = cam;
    }
    inline const game_camera* game_renderer::get_camera() const {
        return m_camera;
    }

    inline void game_renderer::set_viewport(const game_viewport* vp) {
        m_viewport = vp;
    }
    inline const game_viewport* game_renderer::get_viewport() const {
        return m_viewport;
    }

}  // namespace engine
