#pragma once

#ifndef HEALTH_SYSTEM_H
#define HEALTH_SYSTEM_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Timer.hpp>

namespace godot {

    class HealthSystem : public Node {
        GODOT_CLASS(HealthSystem, Node)

    public:
        HealthSystem();
        ~HealthSystem();

        static void _register_methods();

        void _init() override;

        // ============================================================
        // SIGNALS
        // ============================================================
        void emit_max_health_changed(int diff);
        void emit_health_changed(int diff);
        void emit_health_depleted();

        // ============================================================
        // VARIABLES
        // ============================================================
    private:
        int _max_health;
        int _health;
        bool _immortality;

        Timer* immortality_timer;

    public:
        int get_max_health() const;
        void set_max_health(int value);

        int get_health() const;
        void set_health(int value);

        bool get_immortality() const;
        void set_immortality(bool value);

        void set_temporary_immortality(float time);
    };

} // namespace godot

#endif
