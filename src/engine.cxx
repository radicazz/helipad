#include "engine.hxx"

#include <stdexcept>
#include <variant>

#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <laya/events/event_polling.hpp>

#include "logger.hxx"
#include "safety.hxx"

namespace engine {
    game_engine::game_engine(std::string_view title, const glm::ivec2& size, void* game_state,
                             const game_engine_callbacks& callbacks)
        : m_wrapper(),
          m_is_running(false),
          m_state(game_state),
          m_callbacks(callbacks),
          m_window(std::make_unique<game_window>(title, size, game_window_type::resizable)),
          m_renderer(std::make_unique<game_renderer>(m_window->get_laya_window())),
          m_input(std::make_unique<game_input>()),
          m_scenes(std::make_unique<game_scenes>(this)),
          m_tick_interval_seconds(-1.f),
          m_fraction_to_next_tick(-1.f),
          m_frame_interval_seconds(-1.f) {
        // Set a default icon, can be overridden later.
        m_window->set_icon("assets/helipad/icons/default");

        // Set the default tick rate.
        set_tick_rate(32.f);

        // Let the game know it has been created.
        invoke_void(m_callbacks.on_start, this);
    }

    game_engine::~game_engine() {
        invoke_void(m_callbacks.on_end, this);
    }

    void game_engine::start_running() {
        if (m_is_running == true) {
            // TODO: Better way to handle this?
            log_error("Game engine is already running on this object.");
            return;
        }

        m_is_running = true;

        log_info("Starting game loop...");

        std::uint64_t frame_performance_count = performance_counter_value_current();
        float seconds_since_last_tick = 0.f;

        while (m_is_running == true) {
            m_frame_interval_seconds = performance_counter_seconds_since(frame_performance_count);
            frame_performance_count = performance_counter_value_current();
            seconds_since_last_tick += m_frame_interval_seconds;

            {
                m_input->update();

                for (const auto& event : laya::events_view()) {
                    if (std::holds_alternative<laya::quit_event>(event)) {
                        m_is_running = false;
                    }

                    m_input->process_event(event);
                }

                m_scenes->on_engine_input();
            }

            while (seconds_since_last_tick >= m_tick_interval_seconds) [[likely]] {
                m_scenes->on_engine_tick(m_tick_interval_seconds);
                invoke_void(m_callbacks.on_tick, this, m_tick_interval_seconds);
                seconds_since_last_tick -= m_tick_interval_seconds;
            }

            m_fraction_to_next_tick = seconds_since_last_tick / m_tick_interval_seconds;

            m_scenes->on_engine_frame(m_frame_interval_seconds);
            invoke_void(m_callbacks.on_frame, this, m_frame_interval_seconds);

            m_renderer->draw_begin();
            m_scenes->on_engine_draw(m_fraction_to_next_tick);
            invoke_void(m_callbacks.on_draw, this, m_fraction_to_next_tick);
            m_renderer->draw_end();
        }

        log_info("Ending game loop...");
    }

    void game_engine::stop_running() noexcept {
        m_is_running = false;
    }

    game_engine::engine_wrapper::engine_wrapper()
        : m_context(laya::subsystem::video) {
        log_info("\n");
        log_info("Project '{}' (v{} {}) starting up...", project_name, version::full, build_type);

        log_info("SDL initialized successfully: v{}.{}.{}", SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
                 SDL_MICRO_VERSION);

        if (TTF_Init() == false) {
            throw error_message("Failed to initialize SDL_ttf.");
        }

        log_info("TTF initialized successfully: v{}.{}.{}", SDL_TTF_MAJOR_VERSION,
                 SDL_TTF_MINOR_VERSION, SDL_TTF_MICRO_VERSION);
    }

    game_engine::engine_wrapper::~engine_wrapper() {
        TTF_Quit();
        log_info("TTF shut down.");
    }
}  // namespace engine

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        game_entry_point();
    } catch (const std::exception& e) {
        engine::message_box_error("Fatal Error", e.what());
        return 1;
    }

    return 0;
}
