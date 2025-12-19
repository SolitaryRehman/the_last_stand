#programa once 

#ifndef PLAYER2_H
#define PLAYER2_H

#include <Godot.hpp>
#include <CharacterBody2D.hpp>
#include <AnimatedSprite2D.hpp>
#include <Hurtbox.hpp>
#include <Hitbox.hpp>
#include <Timer.hpp>

namespace godot {

    class Player2 : public CharacterBody2D {
        GODOT_CLASS(Player2, CharacterBody2D)

    public:
        // Constructor / Destructor
        Player2();
        ~Player2();

        // Godot lifecycle
        void _init() override;
        void _ready() override;
        void _physics_process(float delta) override;

        // Signals
        static void _register_methods();


        // CONSTANTS

        const float SPEED = 800.0;
        const float JUMP_VELOCITY = -950.0;
        const int BUFFER_SIZE = 10;
        const float BUFFER_DURATION = 0.3;


        // NODES

        Node2D* facing_container;
        AnimatedSprite2D* animated_sprite;

        Hurtbox* hurtbox_standing_area;
        Hurtbox* hurtbox_crouching_area;

        Hitbox* hitbox_punch_area;
        Hitbox* hitbox_kick_area;

        Timer* counter_window_timer;


        // EXPORT VARIABLES

        String character_name;
        int max_health;
        float hitstun_duration;
        float knockdown_duration;
        int damage_amount;


        // STATE VARIABLES

        int health;

        bool is_attacking;
        String current_attack;

        bool is_crouching;
        String crouch_state;

        bool is_stunned;
        float stun_timer;

        bool is_knocked_down;
        float knockdown_time;

        bool is_blocking;
        bool is_blocking_hit;
        bool is_counter_window_active;

        Array<String> input_buffer;
        float buffer_timer;


        // COMBOS

        Dictionary combos;

        // FUNCTIONS

        void _handle_stun(float delta);
        void _handle_knockdown(float delta);
        void _handle_attack_state(float delta);
        void _handle_crouch(bool crouch_pressed);
        void _start_crouch_down();
        void _start_crouch_up();
        void _handle_movement(float delta);
        void _handle_attack_input();
        void _handle_animation();
        void _on_animation_finished();
        void _start_attack(String type);
        void take_damage(int amount, Vector2 hit_position);
        void die();
        void _handle_block();
        void _handle_input_buffer(float delta);
        void add_input_to_buffer(String action);
        void check_for_combos();
        void perform_combo(String combo_name);
        void reset_stats();

        void _on_hurtbox_standing_area_entered(Area2D* area);
        void _on_hurtbox_crouching_area_entered(Area2D* area);

        void _take_punch_hit(Area2D* area);
        void _take_kick_hit(Area2D* area);
    };

} // namespace godot

#endif
