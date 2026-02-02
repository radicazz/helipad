/**
 * @file scene_builder.hxx
 * @brief Builder pattern for game scene creation and configuration.
 *
 * Provides a fluent API for setting up game scenes with callbacks and state.
 */

#pragma once

#include "utils/scenes.hxx"
#include <functional>
#include <string>
#include <optional>

namespace engine {
    /**
     * @brief Builder for creating and configuring game scenes with fluent API.
     *
     * Usage example:
     * @code
     * auto scene = scene_builder("main_scene")
     *     .state(&scene_state)
     *     .on_load([](game_scene& s) { ... })
     *     .on_tick([](game_scene& s, float dt) { ... })
     *     .register_with(engine.get_scenes());
     * @endcode
     */
    class scene_builder {
    public:
        /**
         * @brief Create a scene builder with the specified scene name.
         * @param name Unique name for the scene.
         */
        explicit scene_builder(std::string_view name) : m_name(name) {}

        /**
         * @brief Attach scene-specific state object.
         * @tparam T Type of state object (must be a class).
         * @param state Pointer to state object (ownership not transferred).
         * @return Reference to this builder for chaining.
         */
        template <class T>
            requires std::is_class_v<T>
        scene_builder& state(T* state) {
            m_state = static_cast<void*>(state);
            return *this;
        }

        /**
         * @brief Register callback for scene loading (one-time initialization).
         * @param callback Function called with scene reference.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_load(std::function<void(game_scene&)> callback) {
            m_on_load = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for scene unloading (cleanup).
         * @param callback Function called with scene reference.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_unload(std::function<void(game_scene&)> callback) {
            m_on_unload = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for scene activation (becoming active).
         * @param callback Function called with scene reference.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_activate(std::function<void(game_scene&)> callback) {
            m_on_activate = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for scene deactivation.
         * @param callback Function called with scene reference.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_deactivate(std::function<void(game_scene&)> callback) {
            m_on_deactivate = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for input processing.
         * @param callback Function called with scene reference.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_input(std::function<void(game_scene&)> callback) {
            m_on_input = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for fixed tick updates.
         * @param callback Function called with scene reference and tick interval.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_tick(std::function<void(game_scene&, float)> callback) {
            m_on_tick = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for frame updates (variable rate).
         * @param callback Function called with scene reference and frame interval.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_frame(std::function<void(game_scene&, float)> callback) {
            m_on_frame = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for rendering.
         * @param callback Function called with scene reference and interpolation fraction.
         * @return Reference to this builder for chaining.
         */
        scene_builder& on_draw(std::function<void(game_scene&, float)> callback) {
            m_on_draw = std::move(callback);
            return *this;
        }

        /**
         * @brief Register this scene with the scene manager and optionally activate it.
         * @param scenes Scene manager to register with.
         * @param activate Whether to activate the scene immediately (default: false).
         * @return Name of the registered scene.
         */
        std::string_view register_with(game_scenes* scenes, bool activate = false);

    private:
        std::string m_name;
        void* m_state = nullptr;

        // Callbacks (using std::function for flexibility)
        std::optional<std::function<void(game_scene&)>> m_on_load;
        std::optional<std::function<void(game_scene&)>> m_on_unload;
        std::optional<std::function<void(game_scene&)>> m_on_activate;
        std::optional<std::function<void(game_scene&)>> m_on_deactivate;
        std::optional<std::function<void(game_scene&)>> m_on_input;
        std::optional<std::function<void(game_scene&, float)>> m_on_tick;
        std::optional<std::function<void(game_scene&, float)>> m_on_frame;
        std::optional<std::function<void(game_scene&, float)>> m_on_draw;
    };

    // Internal wrapper struct definition (needs to be accessible to helper function)
    struct scene_state_wrapper {
        void* user_state;
        std::optional<std::function<void(game_scene&)>> on_load;
        std::optional<std::function<void(game_scene&)>> on_unload;
        std::optional<std::function<void(game_scene&)>> on_activate;
        std::optional<std::function<void(game_scene&)>> on_deactivate;
        std::optional<std::function<void(game_scene&)>> on_input;
        std::optional<std::function<void(game_scene&, float)>> on_tick;
        std::optional<std::function<void(game_scene&, float)>> on_frame;
        std::optional<std::function<void(game_scene&, float)>> on_draw;
    };

    /**
     * @brief Helper to retrieve user's actual state from scene built with scene_builder.
     * @tparam T Type of user state.
     * @param scene Scene instance built with scene_builder.
     * @return Pointer to user's state, or nullptr if no state was set.
     */
    template <class T>
        requires std::is_class_v<T>
    [[nodiscard]] T* get_scene_user_state(game_scene& scene) {
        auto* wrapper = static_cast<scene_state_wrapper*>(scene.m_state);
        return static_cast<T*>(wrapper->user_state);
    }
}  // namespace engine
