#include "window.hxx"

#include "../logger.hxx"

#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace engine {
    game_window::game_window(std::string_view title, const glm::ivec2& size, game_window_type type)
        : m_window(nullptr) {
        SDL_WindowFlags window_flags;
        switch (type) {
            case game_window_type::resizable:
                window_flags = SDL_WINDOW_RESIZABLE;
                break;
            case game_window_type::non_resizable:
                window_flags = 0;
                break;
            case game_window_type::borderless:
                window_flags = SDL_WINDOW_BORDERLESS;
                break;
            case game_window_type::fullscreen:
                window_flags = SDL_WINDOW_FULLSCREEN;
                break;
            default:
                window_flags = SDL_WINDOW_RESIZABLE;
                break;
        }

        const std::string title_text{title};
        if (m_window = SDL_CreateWindow(title_text.c_str(), size.x, size.y, window_flags);
            m_window == nullptr) {
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("Failed to create window.");
        }

        log_info("Window created: '{}' ({}x{})", title, size.x, size.y);
    }

    game_window::~game_window() {
        if (m_window != nullptr) {
            SDL_DestroyWindow(m_window);
            log_info("Window destroyed.");
        }
    }

    game_window::game_window(game_window&& other) noexcept : m_window(other.m_window) {
        other.m_window = nullptr;
    }

    game_window& game_window::operator=(game_window&& other) noexcept {
        if (this != &other) {
            if (m_window) {
                SDL_DestroyWindow(m_window);
            }
            m_window = other.m_window;
            other.m_window = nullptr;
        }

        return *this;
    }

    std::string game_window::get_title() const {
        return SDL_GetWindowTitle(m_window);
    }

    void game_window::set_title(std::string_view new_title) {
        const std::string title_text{new_title};
        if (SDL_SetWindowTitle(m_window, title_text.c_str()) == false) {
            log_warning("Failed to set window title: {}", new_title);
            return;
        }

        log_info("Window title set: {}", new_title);
    }

    glm::ivec2 game_window::get_logical_size() const {
        glm::ivec2 size = {0, 0};

        if (SDL_GetWindowSize(m_window, &size.x, &size.y) == false) {
            log_warning("Failed to get window size.");
        }

        return size;
    }

    void game_window::set_logical_size(const glm::ivec2& size) {
        if (SDL_SetWindowSize(m_window, size.x, size.y) == false) {
            log_warning("Failed to set window size: {}x{}", size.x, size.y);
        }
    }

    glm::ivec2 game_window::get_pixel_size() const {
        glm::ivec2 size = {0, 0};

        if (SDL_GetWindowSizeInPixels(m_window, &size.x, &size.y) == false) {
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

            if (SDL_SetWindowIcon(m_window, surface) == false) {
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
