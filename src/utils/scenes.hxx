/**
 * @file scenes.hxx
 * @brief Scene management system for the game engine.
 */

#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <type_traits>

#include "resources.hxx"
#include "../ecs/entities.hxx"
#include "../renderer/camera.hxx"
#include "../renderer/viewport.hxx"

namespace engine {
    class game_scene;

    /**
     * @brief Callbacks for hooking into the game scene lifecycle.
     */
    struct game_scene_callbacks {
        void (*on_load)(game_scene* scene) = nullptr;
        void (*on_unload)(game_scene* scene) = nullptr;
        void (*on_activate)(game_scene* scene) = nullptr;
        void (*on_deactivate)(game_scene* scene) = nullptr;
        void (*on_input)(game_scene* scene) = nullptr;
        void (*on_tick)(game_scene* scene, float tick_interval) = nullptr;
        void (*on_frame)(game_scene* scene, float frame_interval) = nullptr;
        void (*on_draw)(game_scene* scene, float fraction_to_next_tick) = nullptr;
    };

    class game_engine;
    class scene_builder;

    /**
     * @brief Represents a single scene with its own state managed by the engine.
     *
     * @todo Refactor cameras and viewports to use std::vector:
     *      - Both of the classes already store their names as fields.
     *      - No unnecessary string allocations for keys in std::unorderdered_map.
     */
    class game_scene {
        friend scene_builder;
        template <class T>
            requires std::is_class_v<T>
        friend T* get_scene_user_state(game_scene& scene);

    public:
        game_scene() = delete;
        game_scene(std::string_view name, void* state, const game_scene_callbacks& callbacks,
                   game_engine* engine);
        ~game_scene() = default;

        game_scene(const game_scene&) = delete;
        game_scene& operator=(const game_scene&) = delete;
        game_scene(game_scene&&) = default;
        game_scene& operator=(game_scene&&) = default;

        [[nodiscard]] std::string_view get_name() const;

        /**
         * @brief Get the scene-specific state as the specified type.
         * @tparam T The type to cast the scene state to. Must be a class type.
         * @return Pointer to the scene state as type T.
         */
        template <class T>
            requires std::is_class_v<T>
        [[nodiscard]] T* get_state() noexcept;

        [[nodiscard]] game_scene_callbacks& get_callbacks();
        [[nodiscard]] game_engine* get_engine();

        [[nodiscard]] game_entities* get_entities();
        [[nodiscard]] game_resources* get_resources();
        [[nodiscard]] game_camera* get_camera(std::string_view name);
        [[nodiscard]] game_viewport* get_viewport(std::string_view name);

    private:
        std::string m_name;

        void* m_state;
        game_scene_callbacks m_callbacks;
        game_engine* m_engine;

        std::unique_ptr<game_entities> m_entities;
        std::unique_ptr<game_resources> m_resources;
        std::unordered_map<std::string, std::unique_ptr<game_camera>> m_cameras;
        std::unordered_map<std::string, std::unique_ptr<game_viewport>> m_viewports;
    };

    inline std::string_view game_scene::get_name() const {
        return m_name;
    }

    template <class T>
        requires std::is_class_v<T>
    T* game_scene::get_state() noexcept {
        return static_cast<T*>(m_state);
    }

    inline game_scene_callbacks& game_scene::get_callbacks() {
        return m_callbacks;
    }

    inline game_engine* game_scene::get_engine() {
        return m_engine;
    }

    inline game_entities* game_scene::get_entities() {
        return m_entities.get();
    }

    inline game_resources* game_scene::get_resources() {
        return m_resources.get();
    }

    inline game_camera* game_scene::get_camera(std::string_view name) {
        auto it = m_cameras.find(std::string{name});
        if (it != m_cameras.end()) {
            return it->second.get();
        }

        return nullptr;
    }

    inline game_viewport* game_scene::get_viewport(std::string_view name) {
        auto it = m_viewports.find(std::string{name});
        if (it != m_viewports.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * @brief Scene management system for the game engine.
     */
    class game_scenes {
    public:
        game_scenes() = delete;
        explicit game_scenes(game_engine* engine);
        ~game_scenes();

        game_scenes(const game_scenes&) = delete;
        game_scenes& operator=(const game_scenes&) = delete;
        game_scenes(game_scenes&&) = delete;
        game_scenes& operator=(game_scenes&&) = delete;

        void load_scene(std::string_view name, void* state, const game_scene_callbacks& callbacks);
        void unload_scene(std::string_view name);
        [[nodiscard]] bool is_scene_loaded(std::string_view name) const;

        void activate_scene(std::string_view name);
        void deactivate_current_scene();

        [[nodiscard]] bool is_scene_active() const;
        [[nodiscard]] std::string_view get_active_scene_name() const;
        [[nodiscard]] game_scene* get_active_scene();

        void for_each_scene(void (*callback)(std::string_view name, const game_scene& scene)) const;

        void on_engine_tick(float tick_interval);
        void on_engine_frame(float frame_interval);
        void on_engine_draw(float fraction_to_next_tick);
        void on_engine_input();

    private:
        void update_renderer_for_active_scene();
        void reset_renderer_to_global();

    private:
        game_engine* m_engine;
        std::unordered_map<std::string, std::unique_ptr<game_scene>> m_scenes;
        std::string m_active_scene_name;
    };

    inline bool game_scenes::is_scene_loaded(std::string_view name) const {
        return m_scenes.contains(std::string(name));
    }

    inline bool game_scenes::is_scene_active() const {
        return m_active_scene_name.empty() != true;
    }

    inline std::string_view game_scenes::get_active_scene_name() const {
        return m_active_scene_name;
    }

    inline game_scene* game_scenes::get_active_scene() {
        if (is_scene_active() == false) {
            return nullptr;
        }

        auto it = m_scenes.find(std::string(m_active_scene_name));
        if (it != m_scenes.end()) {
            return it->second.get();
        }

        return nullptr;
    }
}  // namespace engine
