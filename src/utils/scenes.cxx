/**
 * @file scenes.cxx
 * @brief Scene management system implementation.
 */

#include "scenes.hxx"

#include <stdexcept>
#include <laya/logging/log.hpp>

#include "../safety.hxx"
#include "../engine.hxx"

namespace engine {
    game_scene::game_scene(std::string_view name, void* state,
                           const game_scene_callbacks& callbacks, game_engine* engine)
        : m_name(name),
          m_state(state),
          m_callbacks(callbacks),
          m_engine(engine),
          m_entities(std::make_unique<game_entities>()),
          m_resources(std::make_unique<game_resources>(engine->get_renderer())),
          m_cameras(),
          m_viewports() {
        paranoid_ensure(name.empty() != true, "Scene name cannot be empty");

        m_cameras[std::string(game_camera::default_name)] =
            std::make_unique<game_camera>(game_camera::default_name, glm::vec2{0.0f, 0.0f}, 1.0f);

        m_viewports[std::string(game_viewport::default_name)] = std::make_unique<game_viewport>(
            game_viewport::default_name, glm::vec2{0.f, 0.f}, glm::vec2{1.f, 1.f});
    }

    game_scenes::game_scenes(game_engine* engine)
        : m_scenes(), m_active_scene_name(), m_engine(engine) {
        paranoid_ensure(m_engine != nullptr, "game_engine pointer cannot be null");
    }

    game_scenes::~game_scenes() {
        for (auto& [scene_id, scene_info] : m_scenes) {
            laya::log_info("Unloading scene '{}' during cleanup", scene_id);
        }

        m_scenes.clear();
    }

    void game_scenes::load_scene(std::string_view name, void* state,
                                 const game_scene_callbacks& callbacks) {
        if (is_scene_loaded(name) == true) {
            laya::log_warn("Scene '{}' is already loaded.", name);
            return;
        }

        auto new_scene = std::make_unique<game_scene>(name, state, callbacks, m_engine);
        game_scene* scene_ptr = new_scene.get();
        m_scenes.emplace(std::string(name), std::move(new_scene));

        invoke_void(scene_ptr->get_callbacks().on_load, scene_ptr);

        laya::log_info("Scene '{}' loaded successfully", name);
    }

    void game_scenes::unload_scene(std::string_view name) {
        if (is_scene_loaded(name) == false) {
            laya::log_warn("Scene '{}' is not loaded.", name);
            return;
        }

        game_scene* scene = m_scenes.at(std::string(name)).get();

        // If this scene is active, deactivate it first.
        if (is_scene_active() == true && m_active_scene_name == name) {
            laya::log_error(
                "Trying to unload active scene '{}'. Set another scene as active before "
                "unloading.",
                name);
            return;
        }

        deactivate_current_scene();
        invoke_void(scene->get_callbacks().on_unload, scene);

        m_scenes.erase(std::string(name));

        laya::log_info("Scene '{}' unloaded successfully", name);
    }

    void game_scenes::activate_scene(std::string_view name) {
        if (is_scene_loaded(name) == false) {
            laya::log_error("Scene '{}' is not loaded. Cannot activate.", name);
            return;
        }

        game_scene* scene = m_scenes.at(std::string(name)).get();

        if (is_scene_active() == true) {
            if (m_active_scene_name == name) {
                laya::log_warn("Scene '{}' is already the active scene", name);
                return;
            }

            deactivate_current_scene();
        }

        invoke_void(scene->get_callbacks().on_activate, scene);

        m_active_scene_name = name;
        update_renderer_for_active_scene();

        laya::log_info("Scene '{}' activated successfully", name);
    }

    void game_scenes::deactivate_current_scene() {
        if (is_scene_active() == false) {
            laya::log_warn("No active scene to deactivate");
            return;
        }

        auto it = m_scenes.find(m_active_scene_name);
        if (it == m_scenes.end()) {
            laya::log_error("Active scene '{}' not found in scene registry", m_active_scene_name);
            m_active_scene_name.clear();
            reset_renderer_to_global();
            return;
        }

        game_scene* scene = it->second.get();
        invoke_void(scene->get_callbacks().on_deactivate, scene);
        m_active_scene_name.clear();
        reset_renderer_to_global();

        laya::log_info("Scene deactivated successfully");
    }

    void game_scenes::for_each_scene(void (*callback)(std::string_view name,
                                                      const game_scene& scene)) const {
        if (callback == nullptr) {
            laya::log_error("Invalid callback provided to for_each_scene");
            return;
        }

        for (const auto& [scene_id, scene_info] : m_scenes) {
            callback(scene_id, *scene_info);
        }
    }

    void game_scenes::on_engine_tick(const float tick_interval) {
        if (game_scene* active_scene = get_active_scene(); active_scene != nullptr) {
            invoke_void(active_scene->get_callbacks().on_tick, active_scene, tick_interval);
        }
    }

    void game_scenes::on_engine_frame(const float frame_interval) {
        if (game_scene* active_scene = get_active_scene(); active_scene != nullptr) {
            invoke_void(active_scene->get_callbacks().on_frame, active_scene, frame_interval);
        }
    }

    void game_scenes::on_engine_draw(const float fraction_to_next_tick) {
        if (game_scene* active_scene = get_active_scene(); active_scene != nullptr) {
            invoke_void(active_scene->get_callbacks().on_draw, active_scene, fraction_to_next_tick);
        }
    }

    void game_scenes::on_engine_input() {
        if (game_scene* active_scene = get_active_scene(); active_scene != nullptr) {
            invoke_void(active_scene->get_callbacks().on_input, active_scene);
        }
    }

    void game_scenes::update_renderer_for_active_scene() {
        if (game_scene* active_scene = get_active_scene(); active_scene != nullptr) {
            if (game_renderer* renderer = m_engine->get_renderer(); renderer != nullptr) {
                if (game_camera* camera = active_scene->get_camera(game_camera::default_name);
                    camera != nullptr) {
                    renderer->set_camera(camera);
                } else {
                    renderer->set_camera(nullptr);
                }

                if (game_viewport* viewport =
                        active_scene->get_viewport(game_viewport::default_name);
                    viewport != nullptr) {
                    renderer->set_viewport(viewport);
                } else {
                    renderer->set_viewport(nullptr);
                }
            }
        }
    }

    void game_scenes::reset_renderer_to_global() {
        game_renderer* renderer = m_engine->get_renderer();
        if (renderer == nullptr) {
            laya::log_error("Cannot reset renderer to global: renderer is null");
            return;
        }

        renderer->set_camera(nullptr);
        renderer->set_viewport(nullptr);
    }
}  // namespace engine
