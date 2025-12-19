#include "CameraController.h"

using namespace godot;

CameraController::CameraController() {}
CameraController::~CameraController() {}

void CameraController::_register_methods() {
	register_method("_process", &CameraController::_process);
	register_property<CameraController, float>("zoom_in", &CameraController::zoom_in, 0.6);
	register_property<CameraController, float>("zoom_out", &CameraController::zoom_out, 0.325);
}

void CameraController::_init() {
	player_1 = nullptr;
	player_2 = nullptr;
	camera_2d = nullptr;

	zoom_in = 0.6;
	zoom_out = 0.325;
}

void CameraController::_process(double delta) {
	if (!player_1 || !player_2 || !camera_2d) return;

	Vector2 p1 = player_1->get_global_position();
	Vector2 p2 = player_2->get_global_position();

	Vector2 midpoint = (p1 + p2) * 0.5;
	camera_2d->set_global_position(midpoint);

	float dist = p1.distance_to(p2);

	float target_zoom_val = clamp(remap(dist, 800, 150, zoom_out, zoom_in), zoom_out, zoom_in);
	Vector2 target_zoom(target_zoom_val, target_zoom_val);

	camera_2d->set_zoom(camera_2d->get_zoom().linear_interpolate(target_zoom, delta * 5.0));
}

// Simple remap function (like Godot's built-in)
float CameraController::remap(float value, float from1, float to1, float from2, float to2) {
	return from2 + (value - from1) * (to2 - from2) / (to1 - from1);
}
