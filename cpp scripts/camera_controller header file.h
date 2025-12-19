#pragma once

#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <Godot.hpp>
#include <Node2D.hpp>
#include <CharacterBody2D.hpp>
#include <Camera2D.hpp>

namespace godot {

    class CameraController : public Node2D {
        GODOT_CLASS(CameraController, Node2D)

    public:
        CameraController();
        ~CameraController();

        static void _register_methods();

        void _init() override;
        void _process(double delta) override;

        // ============================================================
        // NODES
        // ============================================================
        CharacterBody2D* player_1;
        CharacterBody2D* player_2;
        Camera2D* camera_2d;

        // ============================================================
        // EXPORT VARIABLES
        // ============================================================
        float zoom_in;
        float zoom_out;

    private:
        float remap(float value, float from1, float to1, float from2, float to2);
    };

} // namespace godot

#endif

