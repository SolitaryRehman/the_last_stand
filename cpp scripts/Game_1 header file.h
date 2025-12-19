#pragma once

#ifndef MATCH_MANAGER_H
#define MATCH_MANAGER_H

#include <Godot.hpp>
#include <Node2D.hpp>
#include <CharacterBody2D.hpp>
#include <ProgressBar.hpp>
#include <Label.hpp>
#include <Panel.hpp>
#include <Button.hpp>
#include <Timer.hpp>
#include <Tween.hpp>
#include <SceneTree.hpp>

namespace godot {

    class MatchManager : public Node2D {
        GODOT_CLASS(MatchManager, Node2D)

    public:
        MatchManager();
        ~MatchManager();

        static void _register_methods();

        void _init() override;
        void _ready() override;
        void _process(double delta) override;

        // ============================================================
        // NODES
        // ============================================================
        CharacterBody2D* player_1;
        CharacterBody2D* player_2;

        ProgressBar* health_bar_player_1;
        ProgressBar* health_bar_player_2;

        Label* round_timer_label;
        Label* round_counter_label;
        Label* match_state_msgs;

        Timer* round_timer;

        Panel* victory_screen;
        Button* restart_button;

        // ============================================================
        // GAME STATE
        // ============================================================
        enum State { IDLE, INTRO, FIGHT, ROUND_END, MATCH_END };
        State current_state;

        int rounds_won_p1;
        int rounds_won_p2;
        const int MAX_ROUNDS = 3;

        float speed;

        // ============================================================
        // FUNCTIONS
        // ============================================================
        void start_match();
        void start_fight();
        void end_round();
        void reset_round();
        void end_match();

        void on_player_health_changed(int new_health, String character_id);
        void on_character_died(String character_id);
        void on_restart_button_pressed();

        void _on_round_timer_timeout();
    };

} // namespace godot

#endif
