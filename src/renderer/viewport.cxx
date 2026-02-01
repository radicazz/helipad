#include "viewport.hxx"

#include "camera.hxx"
#include "renderer.hxx"

#include <laya/logging/log.hpp>

namespace engine {
    game_viewport::game_viewport(std::string_view name, const glm::vec2& position_normalized,
                                 const glm::vec2& size_normalized)
        : m_name(name),
          m_position(position_normalized),
          m_size(size_normalized),
          m_cached_position_pixels(),
          m_cached_size_pixels() {
        if (m_size.x > 1.f || m_size.x < 0.f) {
            laya::log_warn("Viewport size x component out of range [0.f, 1.f]: {}", m_size.x);
        }

        if (m_size.y > 1.f || m_size.y < 0.f) {
            laya::log_warn("Viewport size y component out of range [0.f, 1.f]: {}", m_size.y);
        }

        set_rect(m_position, m_size);
    }

    void game_viewport::set_rect(const glm::vec2& new_position, const glm::vec2& new_size) {
        set_position(new_position);
        set_size(new_size);
    }

    void game_viewport::apply_to_sdl(game_renderer& renderer) const {
        const glm::vec2 output_size = renderer.get_output_size();

        m_cached_position_pixels = glm::floor(m_position * output_size);
        m_cached_size_pixels = glm::floor(m_size * output_size);

        const laya::rect rect{static_cast<int>(m_cached_position_pixels.x),
                              static_cast<int>(m_cached_position_pixels.y),
                              static_cast<int>(m_cached_size_pixels.x),
                              static_cast<int>(m_cached_size_pixels.y)};

        renderer.get_laya_renderer().set_viewport(rect);
    }

    glm::mat3 game_viewport::get_view_matrix(const game_camera& camera) const {
        // Transform world -> screen inside this viewport:
        // screen = (world - camera_pos) * zoom + viewport_top_left + viewport_size/2
        glm::mat3 view(1.0f);

        const float zoom = camera.get_zoom();

        // Scale
        view[0][0] = zoom;
        view[1][1] = zoom;

        // Translation
        // Center camera in the viewport: origin at viewport center
        const glm::vec2 screen_center = m_cached_position_pixels + m_cached_size_pixels * 0.5f;
        view[2][0] = -camera.get_position().x * zoom + screen_center.x;
        view[2][1] = -camera.get_position().y * zoom + screen_center.y;

        return view;
    }

    glm::vec2 game_viewport::world_to_screen(const game_camera& camera,
                                             const glm::vec2& world_pos) const {
        glm::vec3 world_h = glm::vec3(world_pos, 1.0f);
        glm::vec3 screen_h = get_view_matrix(camera) * world_h;
        return {screen_h.x, screen_h.y};
    }

    glm::vec2 game_viewport::screen_to_world(const game_camera& camera,
                                             const glm::vec2& screen_pos) const {
        const glm::vec2 screen_center = m_cached_position_pixels + m_cached_size_pixels * 0.5f;
        const glm::vec2 centered = screen_pos - screen_center;
        return camera.get_position() + centered / camera.get_zoom();
    }

    std::tuple<glm::vec2, glm::vec2> game_viewport::get_visible_area_world(
        const game_camera& camera) const {
        const glm::vec2 half_viewport_world = (m_cached_size_pixels * 0.5f) / camera.get_zoom();
        const glm::vec2 cam = camera.get_position();

        const float min_x = cam.x - half_viewport_world.x;
        const float min_y = cam.y - half_viewport_world.y;
        const float max_x = cam.x + half_viewport_world.x;
        const float max_y = cam.y + half_viewport_world.y;

        return {glm::vec2(min_x, min_y), glm::vec2(max_x, max_y)};
    }

    bool game_viewport::is_in_view(const game_camera& camera, const glm::vec2& position,
                                   const glm::vec2& size) const {
        const auto [min_bounds, max_bounds] = get_visible_area_world(camera);

        const float obj_left = position.x - size.x * 0.5f;
        const float obj_right = position.x + size.x * 0.5f;
        const float obj_top = position.y - size.y * 0.5f;
        const float obj_bottom = position.y + size.y * 0.5f;

        return !(obj_right < min_bounds.x || obj_left > max_bounds.x || obj_bottom < min_bounds.y ||
                 obj_top > max_bounds.y);
    }

    void game_viewport::clamp_camera_to_bounds(game_camera& camera) const {
        // If the camera has bounds, clamp against half visible size in world units
        const glm::vec2 half_viewport_world = (m_cached_size_pixels * 0.5f) / camera.get_zoom();
        camera.clamp_to_physical_bounds(half_viewport_world);
    }
}  // namespace engine
