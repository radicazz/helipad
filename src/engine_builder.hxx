/**
 * @file engine_builder.hxx
 * @brief Builder pattern for game engine initialization.
 *
 * Provides a fluent API for configuring and constructing the game engine,
 * eliminating boilerplate and improving discoverability.
 */

#pragma once

#include "engine.hxx"
#include <functional>
#include <string>
#include <optional>

namespace engine {
    /**
     * @brief Builder for constructing a game_engine with fluent configuration.
     *
     * Usage example:
     * @code
     * auto engine = engine_builder()
     *     .window("My Game", {1280, 720})
     *     .tick_rate(60.0f)
     *     .on_start([](game_engine& e) { ... })
     *     .build();
     * @endcode
     */
    class engine_builder {
    public:
        engine_builder() = default;

        /**
         * @brief Configure the game window.
         * @param title Window title.
         * @param size Window dimensions in pixels.
         * @return Reference to this builder for chaining.
         */
        engine_builder& window(std::string_view title, const glm::ivec2& size) {
            m_window_title = title;
            m_window_size = size;
            return *this;
        }

        /**
         * @brief Set the fixed update tick rate.
         * @param rate Ticks per second (default: 32.0).
         * @return Reference to this builder for chaining.
         */
        engine_builder& tick_rate(float rate) {
            m_tick_rate = rate;
            return *this;
        }

        /**
         * @brief Register callback for engine startup (after construction).
         * @param callback Function called with engine reference.
         * @return Reference to this builder for chaining.
         */
        engine_builder& on_start(std::function<void(game_engine&)> callback) {
            m_on_start = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for engine shutdown (before destruction).
         * @param callback Function called with engine reference.
         * @return Reference to this builder for chaining.
         */
        engine_builder& on_end(std::function<void(game_engine&)> callback) {
            m_on_end = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for fixed tick updates.
         * @param callback Function called with engine reference and tick interval.
         * @return Reference to this builder for chaining.
         */
        engine_builder& on_tick(std::function<void(game_engine&, float)> callback) {
            m_on_tick = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for frame updates (variable rate).
         * @param callback Function called with engine reference and frame interval.
         * @return Reference to this builder for chaining.
         */
        engine_builder& on_frame(std::function<void(game_engine&, float)> callback) {
            m_on_frame = std::move(callback);
            return *this;
        }

        /**
         * @brief Register callback for rendering.
         * @param callback Function called with engine reference and interpolation fraction.
         * @return Reference to this builder for chaining.
         */
        engine_builder& on_draw(std::function<void(game_engine&, float)> callback) {
            m_on_draw = std::move(callback);
            return *this;
        }

        /**
         * @brief Attach custom state object to the engine.
         * @tparam T Type of state object (must be a class).
         * @param state Pointer to state object (ownership not transferred).
         * @return Reference to this builder for chaining.
         */
        template <class T>
            requires std::is_class_v<T>
        engine_builder& state(T* state) {
            m_state = static_cast<void*>(state);
            return *this;
        }

        /**
         * @brief Build the game engine with configured settings.
         * @return Unique pointer to constructed game_engine.
         * @throws std::runtime_error if required configuration is missing.
         */
        [[nodiscard]] std::unique_ptr<game_engine> build();

        /**
         * @brief Helper to retrieve user's actual state from engine built with builder.
         * @tparam T Type of user state.
         * @param engine Engine instance built with this builder.
         * @return Pointer to user's state, or nullptr if no state was set.
         */
        template <class T>
            requires std::is_class_v<T>
        [[nodiscard]] static T* get_user_state(game_engine& engine);

    private:
        // Forward declare internal wrapper
        struct engine_state_wrapper;

        // Window configuration
        std::string m_window_title = "Helipad Game";
        glm::ivec2 m_window_size = {1280, 720};

        // Engine settings
        std::optional<float> m_tick_rate;

        // State
        void* m_state = nullptr;

        // Callbacks (using std::function for flexibility)
        std::optional<std::function<void(game_engine&)>> m_on_start;
        std::optional<std::function<void(game_engine&)>> m_on_end;
        std::optional<std::function<void(game_engine&, float)>> m_on_tick;
        std::optional<std::function<void(game_engine&, float)>> m_on_frame;
        std::optional<std::function<void(game_engine&, float)>> m_on_draw;
    };

    // Internal wrapper struct definition (must be defined before template function)
    struct engine_builder::engine_state_wrapper {
        void* user_state;
        std::optional<std::function<void(game_engine&)>> on_start;
        std::optional<std::function<void(game_engine&)>> on_end;
        std::optional<std::function<void(game_engine&, float)>> on_tick;
        std::optional<std::function<void(game_engine&, float)>> on_frame;
        std::optional<std::function<void(game_engine&, float)>> on_draw;
    };

    // Template function implementation (must be in header for templates)
    template <class T>
        requires std::is_class_v<T>
    T* engine_builder::get_user_state(game_engine& engine) {
        // Access the wrapper and return the user's actual state
        auto* wrapper = static_cast<engine_state_wrapper*>(engine.m_state);
        return static_cast<T*>(wrapper->user_state);
    }

}  // namespace engine
