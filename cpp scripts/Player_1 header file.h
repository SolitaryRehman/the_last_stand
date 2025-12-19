#pragma once

#ifndef FIGHTER_CHARACTER_H
#define FIGHTER_CHARACTER_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/core/class_db.hpp>
using namespace godot;

class FighterCharacter : public CharacterBody2D {
    GDCLASS(FighterCharacter, CharacterBody2D);

private:
    int health = 100;
    int max_health = 100;
    float speed = 800.0f;
    float jump_velocity = -950.0f;

    AnimatedSprite2D* anim = nullptr;

protected:
    static void _bind_methods();

public:
    FighterCharacter();

    void _ready();
    void _physics_process(double delta);

    void take_damage(int amount);
    void reset_stats();
};

#endif

