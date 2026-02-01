/**
 * @file input.hxx
 * @brief Engine input handling header file.
 */

#pragma once

#include <SDL3/SDL.h>
#include <laya/events/event_types.hpp>
#include <glm/glm.hpp>
#include <unordered_set>
#include <array>
#include <vector>

namespace engine {
    /**
     * @brief Available input system keys.
     */
    enum class game_input_key {
        unknown,

        w,
        a,
        s,
        d,
        c,
        o,
        p,
        g,

        arrow_up,
        arrow_down,
        arrow_left,
        arrow_right,

        space,
        escape,
        enter,

        /**
         * @brief Amount of keys in the enum.
         * @note Should always be above the mouse buttons.
         */
        key_count,

        mouse_left,
        mouse_right,
        mouse_middle
    };

    /**
     * @brief Available key states to query.
     */
    enum class game_input_state { pressed, held, released };

    /**
     * @brief Input system for the game manager.
     */
    class game_input {
        using key_map = std::array<std::pair<SDL_Scancode, game_input_key>,
                                   /*key_count - 2 to exclude itself & unknown*/
                                   static_cast<std::size_t>(game_input_key::key_count) - 2ul>;

    public:
        game_input();
        ~game_input() = default;

        game_input(const game_input&) = default;
        game_input& operator=(const game_input&) = default;
        game_input(game_input&&) = default;
        game_input& operator=(game_input&&) = default;

        void update();
        void process_event(const laya::event& event);

        [[nodiscard]] bool is_key_pressed(game_input_key key) const;
        [[nodiscard]] bool is_key_held(game_input_key key) const;
        [[nodiscard]] bool is_key_released(game_input_key key) const;

        // TODO: Need to make an input mapping system for this...
        // but for now it will suffice.

        [[nodiscard]] glm::vec2 get_movement_wasd() const;
        [[nodiscard]] glm::vec2 get_movement_arrows() const;

        /**
         * @brief Get the current mouse position in screen space.
         * @return The mouse position as a glm::vec2.
         */
        [[nodiscard]] glm::vec2 get_mouse_position() const;

        /**
         * @brief Get the change in mouse movement since the last frame.
         *
         * This function returns the change in mouse position (delta) since the last frame,
         * allowing you to track how much the mouse has moved.
         *
         * @return The mouse movement delta as a glm::vec2.
         */
        [[nodiscard]] glm::vec2 get_mouse_movement() const;
        [[nodiscard]] glm::vec2 get_mouse_wheel() const;
        [[nodiscard]] const std::vector<laya::window_event>& get_window_events() const;

    private:
        [[nodiscard]] constexpr game_input_key sdl_key_to_input_key(SDL_Scancode sdl_key) const;
        [[nodiscard]] constexpr game_input_key sdl_mouse_to_input_key(Uint8 sdl_button) const;

    private:
        std::unordered_set<game_input_key> m_current_keys;
        std::unordered_set<game_input_key> m_previous_keys;
        std::unordered_set<game_input_key> m_pressed_this_frame;
        std::unordered_set<game_input_key> m_released_this_frame;

        glm::vec2 m_mouse_pos;
        glm::vec2 m_mouse_delta;
        glm::vec2 m_previous_mouse_pos;
        glm::vec2 m_mouse_wheel;
        std::vector<laya::window_event> m_window_events;

        static constexpr key_map m_key_map = {
            std::make_pair(SDL_SCANCODE_W, game_input_key::w),
            std::make_pair(SDL_SCANCODE_A, game_input_key::a),
            std::make_pair(SDL_SCANCODE_S, game_input_key::s),
            std::make_pair(SDL_SCANCODE_D, game_input_key::d),
            std::make_pair(SDL_SCANCODE_C, game_input_key::c),
            std::make_pair(SDL_SCANCODE_O, game_input_key::o),
            std::make_pair(SDL_SCANCODE_P, game_input_key::p),
            std::make_pair(SDL_SCANCODE_G, game_input_key::g),
            std::make_pair(SDL_SCANCODE_UP, game_input_key::arrow_up),
            std::make_pair(SDL_SCANCODE_DOWN, game_input_key::arrow_down),
            std::make_pair(SDL_SCANCODE_LEFT, game_input_key::arrow_left),
            std::make_pair(SDL_SCANCODE_RIGHT, game_input_key::arrow_right),
            std::make_pair(SDL_SCANCODE_SPACE, game_input_key::space),
            std::make_pair(SDL_SCANCODE_ESCAPE, game_input_key::escape)};
    };

    inline bool game_input::is_key_pressed(game_input_key k) const {
        return m_pressed_this_frame.find(k) != m_pressed_this_frame.end();
    }

    inline bool game_input::is_key_held(game_input_key k) const {
        return m_current_keys.find(k) != m_current_keys.end();
    }

    inline bool game_input::is_key_released(game_input_key k) const {
        return m_released_this_frame.find(k) != m_released_this_frame.end();
    }

    inline glm::vec2 game_input::get_mouse_position() const {
        return m_mouse_pos;
    }

    inline glm::vec2 game_input::get_mouse_movement() const {
        return m_mouse_delta;
    }

    inline glm::vec2 game_input::get_mouse_wheel() const {
        return m_mouse_wheel;
    }

    inline const std::vector<laya::window_event>& game_input::get_window_events() const {
        return m_window_events;
    }

    constexpr game_input_key game_input::sdl_key_to_input_key(const SDL_Scancode scancode) const {
        for (auto&& pair : m_key_map) {
            if (pair.first == scancode) {
                return pair.second;
            }
        }

        return game_input_key::unknown;
    }

    constexpr game_input_key game_input::sdl_mouse_to_input_key(const Uint8 button) const {
        switch (button) {
            case SDL_BUTTON_LEFT:
                return game_input_key::mouse_left;
            case SDL_BUTTON_RIGHT:
                return game_input_key::mouse_right;
            case SDL_BUTTON_MIDDLE:
                return game_input_key::mouse_middle;
            default:
                return game_input_key::unknown;
        }
    }
}  // namespace engine
