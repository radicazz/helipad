#include "engine_builder.hxx"
#include <stdexcept>

namespace engine {
    std::unique_ptr<game_engine> engine_builder::build() {
        // Create wrapper that stores callbacks and user state
        auto* wrapper = new engine_state_wrapper{
            m_state,
            std::move(m_on_start),
            std::move(m_on_end),
            std::move(m_on_tick),
            std::move(m_on_frame),
            std::move(m_on_draw)
        };

        // Create C-style callback wrappers that retrieve and call std::function objects
        game_engine_callbacks callbacks;

        callbacks.on_start = wrapper->on_start.has_value()
            ? [](game_engine* engine) {
                // Access m_state directly since we're a lambda inside the engine's context
                auto* wrapper = static_cast<engine_state_wrapper*>(engine->m_state);
                (*wrapper->on_start)(*engine);
            }
            : nullptr;

        callbacks.on_end = wrapper->on_end.has_value()
            ? [](game_engine* engine) {
                auto* wrapper = static_cast<engine_state_wrapper*>(engine->m_state);
                (*wrapper->on_end)(*engine);
                delete wrapper;  // Clean up our wrapper
            }
            : [](game_engine* engine) {
                // Always clean up wrapper even if no callback
                auto* wrapper = static_cast<engine_state_wrapper*>(engine->m_state);
                delete wrapper;
            };

        callbacks.on_tick = wrapper->on_tick.has_value()
            ? [](game_engine* engine, float tick_interval) {
                auto* wrapper = static_cast<engine_state_wrapper*>(engine->m_state);
                (*wrapper->on_tick)(*engine, tick_interval);
            }
            : nullptr;

        callbacks.on_frame = wrapper->on_frame.has_value()
            ? [](game_engine* engine, float frame_interval) {
                auto* wrapper = static_cast<engine_state_wrapper*>(engine->m_state);
                (*wrapper->on_frame)(*engine, frame_interval);
            }
            : nullptr;

        callbacks.on_draw = wrapper->on_draw.has_value()
            ? [](game_engine* engine, float fraction_to_next_tick) {
                auto* wrapper = static_cast<engine_state_wrapper*>(engine->m_state);
                (*wrapper->on_draw)(*engine, fraction_to_next_tick);
            }
            : nullptr;

        auto engine = std::make_unique<game_engine>(
            m_window_title,
            m_window_size,
            wrapper,  // Pass wrapper as state
            callbacks
        );

        if (m_tick_rate.has_value()) {
            engine->set_tick_rate(m_tick_rate.value());
        }

        return engine;
    }
}  // namespace engine
