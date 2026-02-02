#include "scene_builder.hxx"

namespace engine {
    std::string_view scene_builder::register_with(game_scenes* scenes, bool activate) {
        // Create wrapper that stores callbacks and user state
        auto* wrapper = new scene_state_wrapper{
            m_state,
            std::move(m_on_load),
            std::move(m_on_unload),
            std::move(m_on_activate),
            std::move(m_on_deactivate),
            std::move(m_on_input),
            std::move(m_on_tick),
            std::move(m_on_frame),
            std::move(m_on_draw)
        };

        // Create C-style callback wrappers
        game_scene_callbacks callbacks;

        callbacks.on_load = wrapper->on_load.has_value()
            ? [](game_scene* scene) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_load)(*scene);
            }
            : nullptr;

        callbacks.on_unload = wrapper->on_unload.has_value()
            ? [](game_scene* scene) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_unload)(*scene);
                delete wrapper;  // Clean up wrapper
            }
            : [](game_scene* scene) {
                // Always clean up wrapper even if no callback
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                delete wrapper;
            };

        callbacks.on_activate = wrapper->on_activate.has_value()
            ? [](game_scene* scene) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_activate)(*scene);
            }
            : nullptr;

        callbacks.on_deactivate = wrapper->on_deactivate.has_value()
            ? [](game_scene* scene) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_deactivate)(*scene);
            }
            : nullptr;

        callbacks.on_input = wrapper->on_input.has_value()
            ? [](game_scene* scene) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_input)(*scene);
            }
            : nullptr;

        callbacks.on_tick = wrapper->on_tick.has_value()
            ? [](game_scene* scene, float tick_interval) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_tick)(*scene, tick_interval);
            }
            : nullptr;

        callbacks.on_frame = wrapper->on_frame.has_value()
            ? [](game_scene* scene, float frame_interval) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_frame)(*scene, frame_interval);
            }
            : nullptr;

        callbacks.on_draw = wrapper->on_draw.has_value()
            ? [](game_scene* scene, float fraction_to_next_tick) {
                auto* wrapper = static_cast<scene_state_wrapper*>(scene->m_state);
                (*wrapper->on_draw)(*scene, fraction_to_next_tick);
            }
            : nullptr;

        // Register the scene with the scene manager
        scenes->load_scene(m_name, wrapper, callbacks);

        if (activate) {
            scenes->activate_scene(m_name);
        }

        return m_name;
    }
}  // namespace engine
