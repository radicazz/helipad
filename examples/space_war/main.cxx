#include <engine.hxx>
#include <engine_builder.hxx>

struct demo_scene_state {
    entt::entity player;
    entt::entity player_label;

    entt::entity asteroid;

    entt::entity camera_mode_text;
    bool is_free_camera;
    float free_camera_speed;
};

void scene_on_load(engine::game_scene* scene) {
    auto* state = scene->get_state<demo_scene_state>();
    engine::game_entities* entities = scene->get_entities();
    engine::game_resources* resources = scene->get_resources();

    // Create a sprite from an image file with the scene's resource manager.
    auto* player_sprite =
        resources->sprite_get_or_create("player_sprite", "assets/space_war/player/default.png");

    // Set the sprite's render/rotation origin to the center of the image.
    player_sprite->set_origin({16, 24});

    // Create player entity in scene's ECS and set up physics components.
    state->player = entities->sprite_create_interpolated("player_sprite");
    entities->set_transform_position(state->player, {200, 200});
    entities->set_velocity_linear_drag(state->player, 0.3f);
    entities->set_velocity_linear_max(state->player, 500.f);
    entities->set_velocity_angular_drag(state->player, 0.3f);
    entities->set_velocity_angular_max(state->player, 360.f);

    // Create some dynamic text that follows the player around and scales with the camera zoom.
    auto* player_label = resources->text_dynamic_get_or_create(
        "player_label", "player", "assets/helipad/fonts/roboto_regular.ttf", 64.0f);
    player_label->set_origin_centered();

    state->player_label = entities->create_text_dynamic("player_label");
    entities->set_transform_position(state->player_label, {200, 230});
    entities->set_transform_scale(state->player_label, {0.25f, 0.25f});

    auto* asteroid_sprite =
        resources->sprite_get_or_create("asteroid_sprite", "assets/space_war/asteroids/ice_1.png");
    asteroid_sprite->set_size({64, 64});
    asteroid_sprite->set_origin(asteroid_sprite->get_size() * 0.5f);
    state->asteroid = entities->sprite_create_interpolated("asteroid_sprite");
    entities->set_transform_position(state->asteroid, {400, 200});
    entities->set_velocity_angular(state->asteroid, 90.f);

    state->is_free_camera = false;
    state->free_camera_speed = 300.f;

    // Some regular UI text.
    resources->text_static_get_or_create("camera_mode_text", "Camera Mode: Follow",
                                         "assets/helipad/fonts/roboto_regular.ttf", 18.0f);
}

void scene_on_tick(engine::game_scene* scene, const float tick_interval) {
    engine::game_entities* entities = scene->get_entities();

    // Update ECS systems at fixed tick rate.
    entities->system_lifetime_update(tick_interval);
    entities->system_physics_update(tick_interval);
}

void scene_on_input(engine::game_scene* scene) {
    auto* state = scene->get_state<demo_scene_state>();
    engine::game_engine* engine = scene->get_engine();
    engine::game_camera* camera = scene->get_camera(engine::game_camera::default_name);
    engine::game_input* input = engine->get_input();

    // Quit the game on Escape key press.
    if (input->is_key_pressed(engine::game_input_key::escape)) {
        engine->stop_running();
    }

    // Toggle camera mode on C key press.
    if (input->is_key_pressed(engine::game_input_key::c)) {
        state->is_free_camera = !state->is_free_camera;
    }

    if (input->is_key_pressed(engine::game_input_key::o)) {
        camera->zoom_additive(-0.2f);
    }

    if (input->is_key_pressed(engine::game_input_key::p)) {
        camera->zoom_additive(0.2f);
    }
}

void scene_on_frame(engine::game_scene* scene, const float frame_interval) {
    auto* state = scene->get_state<demo_scene_state>();
    engine::game_entities* entities = scene->get_entities();
    engine::game_camera* camera = scene->get_camera(engine::game_camera::default_name);
    engine::game_viewport* viewport = scene->get_viewport(engine::game_viewport::default_name);
    engine::game_engine* engine = scene->get_engine();
    engine::game_input* input = engine->get_input();

    constexpr float player_acceleration = 250.f;
    const glm::vec2 movement_input = input->get_movement_wasd();

    if (movement_input.x != 0.0f) {
        entities->add_impulse_right(state->player,
                                    movement_input.x * player_acceleration * frame_interval);
    }

    if (movement_input.y != 0.0f) {
        entities->add_impulse_forward(state->player,
                                      movement_input.y * player_acceleration * frame_interval);
    }

    if (state->is_free_camera == false) {
        const glm::vec2 target_position =
            entities->get_interpolated_position(state->player, engine->get_fraction_to_next_tick());

        camera->follow_target(target_position);
    } else {
        camera->move_position(input->get_movement_arrows() * state->free_camera_speed *
                              frame_interval);
    }

    if (input->is_key_pressed(engine::game_input_key::mouse_left) == true) {
        // Convert mouse position to world space.
        const glm::vec2 mouse_click_position =
            viewport->screen_to_world(*camera, input->get_mouse_position());

        // Move the asteroid to where we clicked.
        entities->set_transform_position(state->asteroid, mouse_click_position);
    }

    // Update player label to follow player
    const glm::vec2 player_position =
        entities->get_interpolated_position(state->player, engine->get_fraction_to_next_tick());
    entities->set_transform_position(state->player_label, player_position + glm::vec2{0.f, 30.f});
}

void scene_on_draw(engine::game_scene* scene, float fraction_to_next_tick) {
    auto* state = scene->get_state<demo_scene_state>();
    engine::game_entities* entities = scene->get_entities();
    engine::game_resources* resources = scene->get_resources();
    engine::game_engine* engine = scene->get_engine();

    // Update and render the camera mode text indicator to the screen (UI overlay).
    if (auto* camera_text = resources->text_static_get("camera_mode_text")) {
        camera_text->set_text("Camera Mode: {}", state->is_free_camera ? "Free" : "Follow");
        camera_text->set_origin_centered();
        const glm::vec2 output_size = engine->get_renderer()->get_output_size();
        engine->get_renderer()->text_draw_screen(camera_text, {output_size.x * 0.5f, 20.f});
    }

    entities->system_renderer_update(engine->get_renderer(), *resources, fraction_to_next_tick);
}

struct demo_engine_state {
    demo_scene_state main_scene;
};

void game_entry_point() {
    auto engine_state = std::make_unique<demo_engine_state>();

    // NEW BUILDER API - More concise and discoverable!
    auto game = engine::engine_builder()
        .window("Space Warfare", {1280, 720})
        .state(engine_state.get())
        .on_start([](engine::game_engine& engine) {
            // Access user state through builder helper
            auto* state = engine::engine_builder::get_user_state<demo_engine_state>(engine);
            engine::game_scenes* scenes = engine.get_scenes();

            scenes->load_scene("main_scene", &state->main_scene,
                              {.on_load = scene_on_load,
                               .on_unload = nullptr,
                               .on_activate = nullptr,
                               .on_deactivate = nullptr,
                               .on_input = scene_on_input,
                               .on_tick = scene_on_tick,
                               .on_frame = scene_on_frame,
                               .on_draw = scene_on_draw});

            scenes->activate_scene("main_scene");
        })
        .on_end([](engine::game_engine& engine) {
            engine.get_scenes()->unload_scene("main_scene");
        })
        .build();

    // Run the game loop. (blocks until the game exits)
    game->start_running();
}

/* OLD API FOR COMPARISON - Can be removed once migration is complete
void game_entry_point() {
    auto engine_state = std::make_unique<demo_engine_state>();

    // Callbacks that the engine exposes to the game developer.
    engine::game_engine_callbacks engine_callbacks = {.on_start = game_on_engine_start,
                                                      .on_end = game_on_engine_end,
                                                      .on_tick = nullptr,
                                                      .on_frame = nullptr,
                                                      .on_draw = nullptr};

    // Create the game and its resources.
    engine::game_engine game("Space Warfare", {1280, 720}, engine_state.get(), engine_callbacks);

    // Run the game loop. (blocks until the game exits)
    game.start_running();
}
*/
