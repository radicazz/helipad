#include "window.hxx"

#include "../logger.hxx"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace engine {
    game_window::game_window(std::string_view title, const glm::ivec2& size, game_window_type type)
        : m_window(title,
                   laya::dimensions{size.x, size.y},
                   [&]() {
                       switch (type) {
                           case game_window_type::resizable:
                               return laya::window_flags::resizable;
                           case game_window_type::non_resizable:
                               return laya::window_flags::none;
                           case game_window_type::borderless:
                               return laya::window_flags::borderless;
                           case game_window_type::fullscreen:
                               return laya::window_flags::fullscreen;
                           default:
                               return laya::window_flags::resizable;
                       }
                   }()),
          m_title(title) {
        log_info("Window created: '{}' ({}x{})", title, size.x, size.y);
    }

    std::string game_window::get_title() const {
        return m_title;
    }

    void game_window::set_title(std::string_view new_title) {
        m_title = std::string(new_title);
        m_window.set_title(m_title);
        log_info("Window title set: {}", m_title);
    }

    glm::ivec2 game_window::get_logical_size() const {
        const laya::dimensions size = m_window.get_size();
        return {size.width, size.height};
    }

    void game_window::set_logical_size(const glm::ivec2& size) {
        m_window.set_size({size.x, size.y});
    }

    glm::ivec2 game_window::get_pixel_size() const {
        glm::ivec2 size = {0, 0};

        if (SDL_GetWindowSizeInPixels(m_window.native_handle(), &size.x, &size.y) == false) {
            log_warning("Failed to get window pixel size.");
        }

        return size;
    }

    void game_window::set_icon(std::string_view icon_path) {
        const std::vector<std::string> icon_paths = {
            std::string(icon_path) + "_48.png",  // Preferred size
            std::string(icon_path) + "_32.png",  // Standard size
            std::string(icon_path) + "_64.png",  // High DPI
            std::string(icon_path) + ".png",     // Fallback
            std::string(icon_path) + ".ico"      // Windows fallback
        };

        for (const auto& path : icon_paths) {
            SDL_Surface* surface = IMG_Load(path.c_str());
            if (surface == nullptr) {
                continue;
            }

            if (SDL_SetWindowIcon(m_window.native_handle(), surface) == false) {
                SDL_DestroySurface(surface);
                log_warning("Failed to set window icon from path: {}", path);
                break;
            }

            SDL_DestroySurface(surface);

            log_info("Window icon set: {}", path);

            return;
        }

        log_warning("Failed to load any icon for path base: {}", icon_path);
    }
}  // namespace engine
