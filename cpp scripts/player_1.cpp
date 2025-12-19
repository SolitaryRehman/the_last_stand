// ==============================
// FighterCharacter.h
// Meteora / Jenova style Godot C++ (GDExtension) - polished, modular
// Note: adapt registration macros to your godot-cpp/GDExtension version if needed.
// ==============================

#pragma once

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class FighterCharacter : public CharacterBody2D {
	GDCLASS(FighterCharacter, CharacterBody2D);

public:
	// Constructor / Destructor
	FighterCharacter();
	~FighterCharacter();

	// Godot lifecycle
	static void _bind_methods();
	void _ready();
	void _physics_process(double delta) override;

	// Public API
	void take_damage(int amount, Vector2 hit_position);
	void reset_stats();

signals:
	// (Note: signal registration happens in _bind_methods)
	void character_died(String character_id);
	void health_changed(int new_health, String character_id);

protected:
	// ============================================================
	// Constants
	// ============================================================
	static constexpr double SPEED = 800.0;
	static constexpr double JUMP_VELOCITY = -950.0;
	static const int BUFFER_SIZE = 10;
	static constexpr double BUFFER_DURATION = 0.3;

	// ============================================================
	// Exported properties (exposed to inspector)
	// ============================================================
	int max_health = 100;
	double hitstun_duration = 0.25;
	double knockdown_duration = 1.0;
	int damage_amount = 10;
	String character_name = "Player_1";

	// ============================================================
	// Node references (set on _ready using get_node)
	// ============================================================
	Node2D *facing_container = nullptr;
	AnimationPlayer *anim_player = nullptr; // Using AnimationPlayer for C++ example

	Area2D *hurtbox_standing = nullptr;
	Area2D *hurtbox_crouching = nullptr;
	Area2D *hitbox_punch = nullptr;
	Area2D *hitbox_kick = nullptr;

	Timer *counter_window_timer = nullptr;

	// ============================================================
	// Runtime state
	// ============================================================
	int health = 100;
	String current_attack = "";
	bool is_attacking = false;

	bool is_crouching = false;
	String crouch_state = "none"; // none/down/idle/up

	bool is_stunned = false;
	double stun_timer = 0.0;
	bool is_knocked_down = false;
	double knockdown_time = 0.0;

	bool is_blocking = false;
	bool is_blocking_hit = false;
	bool is_counter_window_active = false;

	Array input_buffer; // store String action names
	double buffer_timer = 0.0;

	// Simple combo table (String -> Array)
	Dictionary COMBOS;

	// ============================================================
	// Internal helpers
	// ============================================================
	void _handle_stun(double delta);
	void _handle_knockdown(double delta);
	void _handle_attack_state(double delta);
	void _handle_crouch(bool crouch_pressed);
	void _start_crouch_down();
	void _start_crouch_up();
	void _handle_movement(double delta);
	void _handle_attack_input();
	void _handle_animation();
	void _on_animation_finished();

	void _start_attack(const String &type);
	void _take_punch_hit(Area2D *area);
	void _take_kick_hit(Area2D *area);

	void _handle_input_buffer(double delta);
	void add_input_to_buffer(const String &action);
	void check_for_combos();
	void perform_combo(const String &combo_name);
	void _handle_block();
};


// ==============================
// FighterCharacter.cpp
// Implementation file - Meteora style
// ==============================

#include "FighterCharacter.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

// ------------------------------
// Constructor / Destructor
// ------------------------------
FighterCharacter::FighterCharacter() {
	// set sensible defaults
	health = max_health;

	// combo table
	COMBOS["punch_punch_kick"] = Array::make(String("p1_attack_j_simple"), String("p1_attack_j_simple"), String("p1_attack_k_simple"));
	COMBOS["kick_punch"] = Array::make(String("p1_attack_k_simple"), String("p1_attack_j_simple"));
}

FighterCharacter::~FighterCharacter() {
}

// ------------------------------
// Binding
// ------------------------------
void FighterCharacter::_bind_methods() {
	// lifecycle
	ClassDB::bind_method(D_METHOD("_ready"), &FighterCharacter::_ready);
	ClassDB::bind_method(D_METHOD("_physics_process", "delta"), &FighterCharacter::_physics_process);

	// public API
	ClassDB::bind_method(D_METHOD("take_damage", "amount", "hit_position"), &FighterCharacter::take_damage);
	ClassDB::bind_method(D_METHOD("reset_stats"), &FighterCharacter::reset_stats);

	// signals
	ADD_SIGNAL(MethodInfo("character_died", PropertyInfo(Variant::STRING, "character_id")));
	ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "new_health"), PropertyInfo(Variant::STRING, "character_id")));

	// properties (expose typical combat tuning)
	ClassDB::bind_method(D_METHOD("set_max_health", "v"), [](FighterCharacter *s, int v){ s->max_health = v; s->health = v; });
	ClassDB::bind_method(D_METHOD("get_max_health"), [](FighterCharacter *s){ return s->max_health; });
	ClassDB::bind_method(D_METHOD("get_health"), [](FighterCharacter *s){ return s->health; });

	// Note: If you're using GDExtension 1.2+, adapt the registration to the new API.
}

// ------------------------------
// Ready
// ------------------------------
void FighterCharacter::_ready() {
	// Node lookups (adjust node paths in your scene to match)
	facing_container = Object::cast_to<Node2D>(get_node("facing_container"));
	anim_player = Object::cast_to<AnimationPlayer>(get_node("facing_container/AnimationPlayer"));

	hurtbox_standing = Object::cast_to<Area2D>(get_node("facing_container/hurtbox_standing"));
	hurtbox_crouching = Object::cast_to<Area2D>(get_node("facing_container/hurtbox_crouching"));
	hitbox_punch = Object::cast_to<Area2D>(get_node("facing_container/hitbox_punch"));
	hitbox_kick = Object::cast_to<Area2D>(get_node("facing_container/hitbox_kick"));

	// Timer for counter window
	counter_window_timer = memnew(Timer);
	counter_window_timer->set_one_shot(true);
	add_child(counter_window_timer);

	// Connect animation finished if present
	if (anim_player) {
		anim_player->connect("animation_finished", Callable(this, "_on_animation_finished"));
	}

	// Disable hitboxes by default
	if (hitbox_punch) hitbox_punch->set_deferred("monitoring", false);
	if (hitbox_kick) hitbox_kick->set_deferred("monitoring", false);
}

// ------------------------------
// Physics
// ------------------------------
void FighterCharacter::_physics_process(double delta) {
	// STUN
	if (is_stunned && !is_knocked_down) {
		_handle_stun(delta);
		return;
	}

	// KNOCKDOWN
	if (is_knocked_down) {
		_handle_knockdown(delta);
		return;
	}

	// ATTACK LOCK
	if (is_attacking) {
		_handle_attack_state(delta);
		return;
	}

	// NORMAL
	bool crouch_pressed = Input::get_singleton()->is_action_pressed("p1_crouch");
	_handle_crouch(crouch_pressed);
	_handle_movement(delta);
	_handle_attack_input();
	_handle_animation();

	_handle_input_buffer(delta);
	check_for_combos();
	_handle_block();

	// Counter attack input
	if (is_counter_window_active && Input::get_singleton()->is_action_just_pressed("p1_attack_j_simple")) {
		if (is_crouching) _start_attack("counter_crouch_j");
		else _start_attack("counter_standing_j");

		is_counter_window_active = false;
		if (counter_window_timer) counter_window_timer->stop();
	}
}

// ------------------------------
// STUN / KNOCKDOWN
// ------------------------------
void FighterCharacter::_handle_stun(double delta) {
	stun_timer -= delta;
	// damp horizontal movement towards 0
	velocity.x = Math::lerp(velocity.x, 0.0, 400.0 * delta);
	move_and_slide();

	if (stun_timer <= 0.0) {
		is_stunned = false;
		if (anim_player) anim_player->play("p1_idle");
	}
}

void FighterCharacter::_handle_knockdown(double delta) {
	knockdown_time -= delta;
	velocity.x = Math::lerp(velocity.x, 0.0, 50.0 * delta);
	velocity.y += ProjectSettings::get_singleton()->get_setting("physics/2d/default_gravity");
	move_and_slide();

	if (knockdown_time <= 0.0) {
		is_knocked_down = false;
		if (anim_player) anim_player->play("p1_knockdown_get_up");
	}
}

// ------------------------------
// ATTACK STATE
// ------------------------------
void FighterCharacter::_handle_attack_state(double delta) {
	velocity.x = 0;
	if (!is_on_floor()) velocity.y += ProjectSettings::get_singleton()->get_setting("physics/2d/default_gravity");
	else velocity.y = 0;

	move_and_slide();
}

// ------------------------------
// CROUCH
// ------------------------------
void FighterCharacter::_handle_crouch(bool crouch_pressed) {
	if (crouch_pressed && is_on_floor() && !is_attacking) {
		if (crouch_state == "none") { _start_crouch_down(); return; }
		if (crouch_state == "idle") { if (anim_player) anim_player->play("p1_crouch_idle"); velocity.x = 0; }
	}
	else if (!crouch_pressed && is_crouching && !is_attacking) {
		_start_crouch_up();
		return;
	}

	if (hurtbox_standing) hurtbox_standing->set_deferred("monitoring", !is_crouching);
	if (hurtbox_crouching) hurtbox_crouching->set_deferred("monitoring", is_crouching);
}

void FighterCharacter::_start_crouch_down() {
	crouch_state = "down";
	is_crouching = true;
	velocity.x = 0;
	if (anim_player) anim_player->play("p1_crouch_down");

	if (hurtbox_standing) hurtbox_standing->set_deferred("monitoring", false);
	if (hurtbox_crouching) hurtbox_crouching->set_deferred("monitoring", true);
}

void FighterCharacter::_start_crouch_up() {
	crouch_state = "up";
	if (anim_player) anim_player->play("p1_crouch_up");

	if (hurtbox_standing) hurtbox_standing->set_deferred("monitoring", true);
	if (hurtbox_crouching) hurtbox_crouching->set_deferred("monitoring", false);
}

// ------------------------------
// MOVEMENT
// ------------------------------
void FighterCharacter::_handle_movement(double delta) {
	if (!is_on_floor()) {
		if (velocity.y > 0) velocity.y += ProjectSettings::get_singleton()->get_setting("physics/2d/default_gravity") * 3.0 * delta;
		else velocity.y += ProjectSettings::get_singleton()->get_setting("physics/2d/default_gravity") * delta;
	}

	if (Input::get_singleton()->is_action_just_pressed("p1_jump") && is_on_floor()) {
		velocity.y = JUMP_VELOCITY;
	}

	double direction = Input::get_singleton()->get_action_strength("p1_walk_right") - Input::get_singleton()->get_action_strength("p1_walk_left");

	if (direction != 0) {
		if (facing_container) facing_container->set_scale(Vector2(sign(direction), 1));
	}

	if (is_crouching) velocity.x = 0;
	else {
		if (direction != 0) velocity.x = direction * SPEED;
		else velocity.x = Math::lerp(velocity.x, 0.0, SPEED * delta);
	}

	move_and_slide();
}

// ------------------------------
// ATTACK INPUT
// ------------------------------
void FighterCharacter::_handle_attack_input() {
	if (is_attacking) return;

	double direction = Input::get_singleton()->get_action_strength("p1_walk_right") - Input::get_singleton()->get_action_strength("p1_walk_left");

	if (Input::get_singleton()->is_action_just_pressed("p1_attack_j_simple")) {
		if (is_crouching) { if (direction != 0) _start_attack("crouch_side_j"); else _start_attack("crouch_j"); return; }
		if (!is_on_floor()) { if (direction != 0) _start_attack("jump_side_j"); else _start_attack("jump_j"); return; }
		if (direction != 0) { _start_attack("simple_side_j"); return; }
		_start_attack("simple_j");
	}

	if (Input::get_singleton()->is_action_just_pressed("p1_attack_k_simple")) {
		if (is_crouching) { if (direction != 0) _start_attack("crouch_side_k"); else _start_attack("crouch_k"); return; }
		if (!is_on_floor()) { if (direction != 0) _start_attack("jump_side_k"); else _start_attack("jump_k"); return; }
		if (direction != 0) { _start_attack("simple_side_k"); return; }
		_start_attack("simple_k");
	}
}

// ------------------------------
// ANIMATION HANDLING
// ------------------------------
void FighterCharacter::_handle_animation() {
	if (is_attacking || is_stunned) return;

	if (is_on_floor()) {
		if (crouch_state == "idle") { if (anim_player) anim_player->play("p1_crouch_idle"); return; }
		if (crouch_state == "up" || crouch_state == "down") return;
		double dir = Input::get_singleton()->get_action_strength("p1_walk_right") - Input::get_singleton()->get_action_strength("p1_walk_left");
		if (dir == 0) { if (anim_player) anim_player->play("p1_idle"); }
		else { if (anim_player) anim_player->play("p1_walk"); }
	}
	else {
		if (anim_player) anim_player->play("p1_jump");
	}
}

void FighterCharacter::_on_animation_finished() {
	// Example: when attack animation finishes
	// For robust usage, inspect the animation name via AnimationPlayer API
	// This method should be expanded with your chosen animation naming conventions.
	is_attacking = false;
	current_attack = "";

	if (hitbox_punch) hitbox_punch->set_deferred("monitoring", false);
	if (hitbox_kick) hitbox_kick->set_deferred("monitoring", false);

	if (is_crouching && !Input::get_singleton()->is_action_pressed("p1_crouch")) {
		_start_crouch_up();
	}
}

// ------------------------------
// ATTACK START
// ------------------------------
void FighterCharacter::_start_attack(const String &type) {
	is_attacking = true;

	if (type == "crouch_j") current_attack = "p1_attack_crouch_j_simple";
	else if (type == "crouch_side_j") current_attack = "p1_attack_crouch_j_side";
	else if (type == "jump_j") current_attack = "p1_attack_jump_j_simple";
	else if (type == "jump_side_j") current_attack = "p1_attack_jump_j_side";
	else if (type == "simple_side_j") current_attack = "p1_attack_j_side";
	else if (type == "simple_j") current_attack = "p1_attack_j_simple";

	else if (type == "counter_standing_j") current_attack = "p1_standing_block_counter";
	else if (type == "counter_crouch_j") current_attack = "p1_crouch_block_counter";

	else if (type == "crouch_k") current_attack = "p1_attack_crouch_k_simple";
	else if (type == "crouch_side_k") current_attack = "p1_attack_crouch_k_side";
	else if (type == "jump_k") current_attack = "p1_attack_jump_k_simple";
	else if (type == "jump_side_k") current_attack = "p1_attack_jump_k_side";
	else if (type == "simple_side_k") current_attack = "p1_attack_k_side";
	else if (type == "simple_k") current_attack = "p1_attack_k_simple";

	// enable hitboxes based on type
	if (type.find("j") != -1) if (hitbox_punch) hitbox_punch->set_deferred("monitoring", true);
	if (type.find("k") != -1) if (hitbox_kick) hitbox_kick->set_deferred("monitoring", true);

	if (anim_player) anim_player->play(current_attack);
}

// ------------------------------
// DAMAGE SYSTEM
// ------------------------------
void FighterCharacter::take_damage(int amount, Vector2 hit_position) {
	if (is_knocked_down) return;

	int final_damage = amount;
	double knockback_force = 700.0;
	bool play_get_hit = true;
	bool apply_stun = true;

	if (is_blocking) {
		final_damage = int(amount * 0.2);
		play_get_hit = false;
		apply_stun = false;

		is_counter_window_active = true;
		if (counter_window_timer) counter_window_timer->start(0.2);
	}

	health -= final_damage;
	health = CLAMP(health, 0, max_health);

	// emit signal
	// Note: in C++ signal emission differs by version — you may need to use a proper API or call a callback.
	// For editor preview we call UtilityFunctions::print
	UtilityFunctions::print(String("health changed: ") + String::num_int64(health));

	// knockback
	double dir = Math::sign(global_position.x - hit_position.x);
	velocity.x = dir * knockback_force;
	velocity.y = -10;

	if (final_damage >= 25) {
		is_knocked_down = true;
		knockdown_time = knockdown_duration;
		if (anim_player) anim_player->play("p1_knockdown");
		return;
	}

	if (apply_stun) {
		is_stunned = true;
		stun_timer = hitstun_duration;
		is_attacking = false;
		if (play_get_hit) {
			if (is_crouching) if (anim_player) anim_player->play("p1_get_hit_crouch");
			else if (anim_player) anim_player->play("p1_get_hit");
		}
	}
	else {
		if (is_crouching) if (anim_player) anim_player->play("p1_block_crouch");
		else if (anim_player) anim_player->play("p1_block_standing");
	}

	if (health <= 0) {
		if (anim_player) anim_player->play("p1_defeat");
		// emit character_died signal or call a callback
		UtilityFunctions::print(String("Character died: ") + character_name);
	}
}

// ------------------------------
// Hit handlers (example)
// ------------------------------
void FighterCharacter::_take_punch_hit(Area2D *area) {
	if (!area) return;
	take_damage(10, area->get_global_position());
	is_stunned = true;
	stun_timer = hitstun_duration;
	if (is_crouching) if (anim_player) anim_player->play("p1_get_hit_crouch");
	else if (anim_player) anim_player->play("p1_get_hit");
}

void FighterCharacter::_take_kick_hit(Area2D *area) {
	if (!area) return;
	take_damage(5, area->get_global_position());
	is_knocked_down = true;
	knockdown_time = knockdown_duration;

	velocity.y = -200;
	double dir = Math::sign(global_position.x - area->get_global_position().x);
	velocity.x = dir * 1300.0;
	if (anim_player) anim_player->play("p1_knockdown");
}

// ------------------------------
// Input buffer & combos
// ------------------------------
void FighterCharacter::_handle_input_buffer(double delta) {
	buffer_timer += delta;

	if (Input::get_singleton()->is_action_just_pressed("p1_attack_j_simple")) add_input_to_buffer("p1_attack_j_simple");
	if (Input::get_singleton()->is_action_just_pressed("p1_jump")) add_input_to_buffer("p1_jump");

	if (buffer_timer >= BUFFER_DURATION && input_buffer.size() > 0) {
		input_buffer.remove(0);
		buffer_timer = 0.0;
	}
}

void FighterCharacter::add_input_to_buffer(const String &action) {
	input_buffer.append(action);
	if (input_buffer.size() > BUFFER_SIZE) input_buffer.remove(0);
	buffer_timer = 0.0;
}

void FighterCharacter::check_for_combos() {
	if (is_attacking || is_stunned) return;

	Array keys = COMBOS.keys();
	for (int i = 0; i < keys.size(); ++i) {
		String combo_name = keys[i];
		Array seq = COMBOS[combo_name];
		if (input_buffer.size() >= seq.size()) {
			int start = input_buffer.size() - seq.size();
			Array slice;
			for (int j = 0; j < seq.size(); ++j) slice.append(input_buffer[start + j]);
			if (slice == seq) { perform_combo(combo_name); input_buffer.clear(); return; }
		}
	}
}

void FighterCharacter::perform_combo(const String &combo_name) {
	if (combo_name == "punch_punch_kick") {
		if (anim_player) anim_player->play("combo_punch_punch_kick");
	}
	else if (combo_name == "kick_punch") {
		if (anim_player) anim_player->play("combo_kick_punch");
	}

	is_attacking = true;
	if (hitbox_punch) hitbox_punch->set_deferred("monitoring", true);
	if (hitbox_kick) hitbox_kick->set_deferred("monitoring", true);
}

// ------------------------------
// Block
// ------------------------------
void FighterCharacter::_handle_block() {
	if (Input::get_singleton()->is_action_pressed("p1_block_standing") && is_on_floor() && !is_attacking) {
		if (!is_blocking) { is_blocking = true; if (is_crouching) if (anim_player) anim_player->play("p1_block_crouch_start"); else if (anim_player) anim_player->play("p1_block_standing_start"); }
		else { if (is_crouching) if (anim_player) anim_player->play("p1_block_crouch_idle"); else if (anim_player) anim_player->play("p1_block_standing_idle"); }
	}
	else {
		if (is_blocking) { is_blocking = false; if (is_crouching) if (anim_player) anim_player->play("p1_block_crouch_release"); else if (anim_player) anim_player->play("p1_block_standing_release"); }
	}
}

// ------------------------------
// Reset stats
// ------------------------------
void FighterCharacter::reset_stats() {
	health = max_health;
	is_stunned = false;
	is_attacking = false;
	is_blocking = false;
	velocity = Vector2(0,0);
	UtilityFunctions::print("Stats reset");
}

// End of file
