#include "resources.hxx"

#include <stdexcept>
#include <format>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "../logger.hxx"
#include "../safety.hxx"

#include "../renderer/renderer.hxx"

namespace engine {
    game_resources::game_resources(game_renderer* renderer)
        : m_textures(),
          m_sprites(),
          m_fonts(),
          m_static_texts(),
          m_dynamic_texts(),
          m_renderer(renderer) {
        paranoid_ensure(m_renderer != nullptr, "game_renderer pointer cannot be null");
    }

    game_resources::~game_resources() {
        texts_clear();
        sprites_clear();
        textures_clear();
        fonts_clear();
    }

    game_resources::game_resources(game_resources&& other) noexcept
        : m_textures(std::move(other.m_textures)),
          m_sprites(std::move(other.m_sprites)),
          m_fonts(std::move(other.m_fonts)),
          m_static_texts(std::move(other.m_static_texts)),
          m_dynamic_texts(std::move(other.m_dynamic_texts)),
          m_renderer(other.m_renderer) {
    }

    game_resources& game_resources::operator=(game_resources&& other) noexcept {
        if (this != &other) {
            texts_clear();
            sprites_clear();
            textures_clear();
            fonts_clear();

            // Move resources (reference stays the same)
            m_textures = std::move(other.m_textures);
            m_sprites = std::move(other.m_sprites);
            m_fonts = std::move(other.m_fonts);
            m_static_texts = std::move(other.m_static_texts);
            m_dynamic_texts = std::move(other.m_dynamic_texts);
            m_renderer = other.m_renderer;
        }

        return *this;
    }

    game_sprite* game_resources::sprite_get_or_create(std::string_view key,
                                                      std::string_view file_path) {
        auto it = m_sprites.find(std::string(key));
        if (it != m_sprites.end()) {
            return it->second.get();
        }

        SDL_Texture* texture = texture_get_or_create(file_path);
        auto sprite = std::make_unique<game_sprite>(file_path, texture);
        auto* sprite_ptr = sprite.get();
        m_sprites[std::string(key)] = std::move(sprite);

        log_info("Created sprite: {}", key);

        return sprite_ptr;
    }

    game_sprite* game_resources::sprite_get(std::string_view key) {
        auto it = m_sprites.find(std::string(key));
        return (it != m_sprites.end()) ? it->second.get() : nullptr;
    }

    void game_resources::sprite_destroy(std::string_view key) {
        auto it = m_sprites.find(std::string(key));
        if (it != m_sprites.end()) {
            log_info("Destroyed sprite: {}", key);
            m_sprites.erase(it);
        }
    }

    void game_resources::textures_clear() {
        for (auto& [key, texture] : m_textures) {
            SDL_DestroyTexture(texture);
            log_info("Destroyed texture: {}", key);
        }

        m_textures.clear();
    }

    void game_resources::fonts_clear() {
        for (auto& [key, font] : m_fonts) {
            TTF_CloseFont(font);
            log_info("Destroyed font: {}", key);
        }

        m_fonts.clear();
    }

    SDL_Texture* game_resources::texture_get_or_create(std::string_view file_path) {
        const std::string key{file_path};
        if (is_texture_loaded(key) == true) {
            return m_textures.at(key);
        }

        SDL_Texture* texture = IMG_LoadTexture(m_renderer->get_sdl_renderer(), key.c_str());
        if (texture == nullptr) {
            throw error_message("Failed to load the texture at: {}", file_path);
        }

        m_textures[key] = texture;

        log_info("Loaded texture: {}", file_path);

        return texture;
    }

    void game_resources::texture_destroy(std::string_view file_path) {
        const std::string key{file_path};
        auto it = m_textures.find(key);
        if (it != m_textures.end()) {
            SDL_DestroyTexture(it->second);
            log_info("Unloaded texture: {}", file_path);
            m_textures.erase(it);
        }
    }

    bool game_resources::is_texture_loaded(std::string_view file_path) const {
        return m_textures.contains(std::string(file_path));
    }

    TTF_Font* game_resources::font_get_or_create(std::string_view font_path, float font_size) {
        std::string unique_key = get_font_unique_key(font_path, font_size);

        if (is_font_loaded(unique_key) == true) {
            log_info("Using cached font: {}", unique_key);
            return m_fonts.at(unique_key);
        }

        const std::string path{font_path};
        TTF_Font* font = TTF_OpenFont(path.c_str(), font_size);
        if (font == nullptr) {
            throw error_message("Failed to load font: {}", font_path);
        }

        m_fonts[unique_key] = font;

        log_info("Loaded font: {} (size: {})", font_path, font_size);

        return font;
    }

    void game_resources::font_destroy(std::string_view unique_key) {
        const std::string key{unique_key};
        auto it = m_fonts.find(key);
        if (it != m_fonts.end()) {
            TTF_CloseFont(it->second);
            log_info("Unloaded font: {}", unique_key);
            m_fonts.erase(it);
        }
    }

    bool game_resources::is_font_loaded(std::string_view unique_key) const {
        return m_fonts.contains(std::string(unique_key));
    }

    std::string game_resources::get_font_unique_key(std::string_view font_path,
                                                    float font_size) const {
        return std::format("{}:{}", font_path, font_size);
    }

    game_text_static* game_resources::text_static_get_or_create(std::string_view key,
                                                                std::string_view text,
                                                                std::string_view font_path,
                                                                float font_size) {
        auto it = m_static_texts.find(std::string(key));
        if (it != m_static_texts.end()) {
            return it->second.get();
        }

        TTF_Font* font = font_get_or_create(font_path, font_size);
        TTF_Text* sdl_text =
            TTF_CreateText(m_renderer->get_sdl_text_engine(), font, text.data(), text.length());
        if (sdl_text == nullptr) {
            throw error_message("Failed to create static text: {}", key);
        }

        auto text_obj = std::make_unique<game_text_static>(sdl_text);
        game_text_static* ptr = text_obj.get();
        m_static_texts[std::string(key)] = std::move(text_obj);

        log_info("Created static text resource: {}", key);
        return ptr;
    }
    game_text_dynamic* game_resources::text_dynamic_get_or_create(std::string_view key,
                                                                  std::string_view initial_text,
                                                                  std::string_view font_path,
                                                                  float font_size) {
        auto it = m_dynamic_texts.find(std::string(key));
        if (it != m_dynamic_texts.end()) {
            return it->second.get();
        }

        TTF_Font* font = font_get_or_create(font_path, font_size);
        TTF_Text* sdl_text = TTF_CreateText(m_renderer->get_sdl_text_engine(), font,
                                            initial_text.data(), initial_text.length());
        if (sdl_text == nullptr) {
            throw std::runtime_error(std::format("Failed to create dynamic text base: {}", key));
        }

        auto text_obj = std::make_unique<game_text_dynamic>(std::string(initial_text), sdl_text,
                                                            m_renderer->get_sdl_renderer(), font);
        game_text_dynamic* ptr = text_obj.get();
        m_dynamic_texts[std::string(key)] = std::move(text_obj);

        log_info("Created dynamic text resource: {}", key);
        return ptr;
    }

    game_text_static* game_resources::text_static_get(std::string_view key) {
        auto it = m_static_texts.find(std::string(key));
        return (it != m_static_texts.end()) ? it->second.get() : nullptr;
    }

    game_text_dynamic* game_resources::text_dynamic_get(std::string_view key) {
        auto it = m_dynamic_texts.find(std::string(key));
        return (it != m_dynamic_texts.end()) ? it->second.get() : nullptr;
    }

    void game_resources::text_static_destroy(std::string_view key) {
        auto it = m_static_texts.find(std::string(key));
        if (it != m_static_texts.end()) {
            log_info("Unloaded static text: {}", key);
            m_static_texts.erase(it);
        }
    }

    void game_resources::text_dynamic_destroy(std::string_view key) {
        auto it = m_dynamic_texts.find(std::string(key));
        if (it != m_dynamic_texts.end()) {
            log_info("Unloaded dynamic text: {}", key);
            m_dynamic_texts.erase(it);
        }
    }

    void game_resources::sprites_clear() {
        log_info("Unloading {} sprite resources.", m_sprites.size());
        m_sprites.clear();
    }

    void game_resources::texts_clear() {
        log_info("Unloading {} static text resources.", m_static_texts.size());
        m_static_texts.clear();

        log_info("Unloading {} dynamic text resources.", m_dynamic_texts.size());
        m_dynamic_texts.clear();
    }
}  // namespace engine
