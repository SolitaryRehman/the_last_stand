#pragma once

#ifndef HEALTH_BAR_H
#define HEALTH_BAR_H

#include <Godot.hpp>
#include <ProgressBar.hpp>
#include <Timer.hpp>

namespace godot {

    class HealthBar : public ProgressBar {
        GODOT_CLASS(HealthBar, ProgressBar)

    public:
        HealthBar();
        ~HealthBar();

        static void _register_methods();

        void _init() override;
        void _process(double delta) override;

        // ============================================================
        // NODES
        // ============================================================
        ProgressBar* damage_bar;
        Timer* timer;

        // ============================================================
        // VARIABLES
        // ============================================================
        int health;

        // ============================================================
        // METHODS
        // ============================================================
        void _set_health(int new_health);
        void init_health(int _health);
        void _on_timer_timeout();
    };

} // namespace godot

#endif
