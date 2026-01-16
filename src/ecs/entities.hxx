/**
 * @file entities.hxx
 * @brief Header file for ECS wrapper that manages its own registry.
 */

#pragma once

#include <string_view>
#include <entt/entt.hpp>
#include "systems.hxx"
#include "components.hxx"

namespace engine {
    class game_renderer;
    class game_resources;

    /**
     * @brief ECS wrapper that manages its own registry.
     */
    class game_entities {
    public:
        game_entities() = default;
        ~game_entities() = default;

        // Registry is non-copyable
        game_entities(const game_entities&) = delete;
        game_entities& operator=(const game_entities&) = delete;
        game_entities(game_entities&&) = default;
        game_entities& operator=(game_entities&&) = default;

        // Registry access
        [[nodiscard]] entt::registry& registry();
        [[nodiscard]] const entt::registry& registry() const;

        // System updates
        void system_physics_update(float tick_interval);
        void system_lifetime_update(float tick_interval);
        void system_renderer_update(game_renderer* renderer, game_resources& resources,
                                    float fraction_to_next_tick);

        [[nodiscard]] entt::entity create();
        void destroy(entt::entity entity);

        [[nodiscard]] bool is_valid(entt::entity entity) const;
        void clear();

        entt::entity sprite_create(std::string_view resource_key);
        entt::entity sprite_create_interpolated(std::string_view resource_key);
        entt::entity create_text_dynamic(std::string_view resource_key);

        // Component access - simplified API
        template <typename Component>
        Component& get(entt::entity entity);

        template <typename Component>
        const Component& get(entt::entity entity) const;

        template <typename Component>
        Component* try_get(entt::entity entity);

        template <typename Component>
        const Component* try_get(entt::entity entity) const;

        template <typename Component, typename... Args>
        Component& add(entt::entity entity, Args&&... args);

        template <typename Component>
        void remove(entt::entity entity);

        template <typename... Components>
        bool has(entt::entity entity) const;

        // Query methods
        template <typename... Components>
        auto view();

        void set_transform_position(entt::entity entity, const glm::vec2& position);
        glm::vec2 get_transform_position(entt::entity entity) const;

        [[nodiscard]] glm::vec2 get_interpolated_position(entt::entity entity,
                                                          float fraction_to_next_tick) const;
        [[nodiscard]] float get_interpolated_rotation(entt::entity entity,
                                                      float fraction_to_next_tick) const;

        void set_transform_scale(entt::entity entity, const glm::vec2& new_scale);
        [[nodiscard]] glm::vec2 get_transform_scale(entt::entity entity);

        /**
         * @brief Get the forward vector (unit vector) based on the entity's rotation.
         * @param entity The entity to get the forward vector for.
         * @return The forward vector as a glm::vec2.
         * @note Default forward is (0, 1) (+Y) when rotation is 0 degrees.
         */
        [[nodiscard]] glm::vec2 get_vector_forward(entt::entity entity) const;

        /**
         * @brief Get the right vector (unit vector) based on the entity's rotation.
         * @param entity The entity to get the right vector for.
         * @return The right vector as a glm::vec2.
         * @note Default right is (1, 0) (+X) when rotation is 0 degrees.
         */
        [[nodiscard]] glm::vec2 get_vector_right(entt::entity entity) const;

        void add_impulse_forward(entt::entity entity, float magnitude);
        void add_impulse_backward(entt::entity entity, float magnitude);
        void add_impulse_right(entt::entity entity, float magnitude);
        void add_impulse_left(entt::entity entity, float magnitude);

        void add_impulse_direction(entt::entity entity, float angle_degrees, float magnitude);
        void add_impulse_relative(entt::entity entity, const glm::vec2& relative_direction,
                                  float magnitude);

        void set_velocity_linear(entt::entity entity, const glm::vec2& velocity);
        void add_impulse_velocity_linear(entt::entity entity, const glm::vec2& impulse);
        void set_velocity_linear_drag(entt::entity entity, float linear_drag);
        void set_velocity_linear_max(entt::entity entity, float max_speed);

        void set_velocity_angular(entt::entity entity, float angular_velocity);
        void add_impulse_velocity_angular(entt::entity entity, float angular_impulse);
        void set_velocity_angular_drag(entt::entity entity, float angular_drag);
        void set_velocity_angular_max(entt::entity entity, float max_angular_speed);

        void set_renderable_visible(entt::entity entity, bool is_visible);
        void set_renderable_layer(entt::entity entity, int layer);

    private:
        entt::registry m_registry;
    };

    // Inline implementations
    inline entt::registry& game_entities::registry() {
        return m_registry;
    }

    inline const entt::registry& game_entities::registry() const {
        return m_registry;
    }

    inline entt::entity game_entities::create() {
        return m_registry.create();
    }

    inline void game_entities::destroy(entt::entity entity) {
        if (m_registry.valid(entity)) {
            m_registry.destroy(entity);
        }
    }

    inline bool game_entities::is_valid(entt::entity entity) const {
        return m_registry.valid(entity);
    }

    inline void game_entities::clear() {
        m_registry.clear();
    }

    template <typename Component>
    inline Component& game_entities::get(entt::entity entity) {
        return m_registry.get<Component>(entity);
    }

    template <typename Component>
    inline const Component& game_entities::get(entt::entity entity) const {
        return m_registry.get<Component>(entity);
    }

    template <typename Component>
    inline Component* game_entities::try_get(entt::entity entity) {
        return m_registry.try_get<Component>(entity);
    }

    template <typename Component>
    inline const Component* game_entities::try_get(entt::entity entity) const {
        return m_registry.try_get<Component>(entity);
    }

    template <typename Component, typename... Args>
    inline Component& game_entities::add(entt::entity entity, Args&&... args) {
        return m_registry.emplace<Component>(entity, std::forward<Args>(args)...);
    }

    template <typename Component>
    inline void game_entities::remove(entt::entity entity) {
        m_registry.remove<Component>(entity);
    }

    template <typename... Components>
    inline bool game_entities::has(entt::entity entity) const {
        return m_registry.all_of<Components...>(entity);
    }

    template <typename... Components>
    inline auto game_entities::view() {
        return m_registry.view<Components...>();
    }

}  // namespace engine
