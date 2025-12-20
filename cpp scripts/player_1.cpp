#pragma once

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;


struct VitalStats {
	int max_health = 100;
	int health = 100;
};

struct CombatState {
	bool attacking = false;
	bool stunned = false;
	bool knocked_down = false;
	bool blocking = false;
	bool counter_window = false;
};

struct Timers {
	double stun = 0.0;
	double knockdown = 0.0;
	double counter = 0.0;
};

struct MovementState {
	bool crouching = false;
	bool airborne = false;
	String crouch_phase = "none"; // none/down/idle/up
};

struct NodeRefs {
	Node2D *facing = nullptr;
	AnimationPlayer *anim = nullptr;

	Area2D *hitboxes[2] = { nullptr, nullptr };
	Area2D *hurtboxes[2] = { nullptr, nullptr };

	Timer *counter_timer = nullptr;
};


class FighterCharacter : public CharacterBody2D {
	GDCLASS(FighterCharacter, CharacterBody2D);

public:
	FighterCharacter();
	~FighterCharacter();

	void _ready();
	void _physics_process(double delta);

	void take_damage(int amount, Vector2 hit_pos);
	void reset_stats();

protected:
	static void _bind_methods();

private:
	// ================= CORE STATE =================
	VitalStats vitals;
	CombatState combat;
	Timers timers;
	MovementState movement;
	NodeRefs nodes;

	Dictionary combos;
	Array input_buffer;

	// ================= CORE SYSTEMS =================
	void process_state(double delta);
	void process_movement(double delta);
	void process_combat(double delta);
	void process_animation();
	void process_blocking();
	void process_buffer(double delta);

	// ================= ATTACK SYSTEM =================
	void start_attack(const String &id);
	void stop_attack();
	void toggle_hitboxes(bool punch, bool kick);

	// ================= COMBO SYSTEM =================
	bool match_combo_recursive(const Array &combo, int combo_i, int buffer_i);
	void check_combos();

	// ================= DAMAGE SYSTEM =================
	void apply_knockback(Vector2 hit_pos, double force);
	void apply_stun(double duration);

	// ================= HELPERS =================
	void apply_gravity(double delta);
	void safe_play(const String &anim);
	bool can_accept_input() const;
};


FighterCharacter::FighterCharacter() {
	vitals.health = vitals.max_health;

	// Register combos
	combos["ppk"] = Array::make(
		"p1_attack_j_simple",
		"p1_attack_j_simple",
		"p1_attack_k_simple"
	);
}

FighterCharacter::~FighterCharacter() {
}

void FighterCharacter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_ready"), &FighterCharacter::_ready);
	ClassDB::bind_method(D_METHOD("_physics_process", "delta"), &FighterCharacter::_physics_process);
	ClassDB::bind_method(D_METHOD("take_damage", "amount", "hit_pos"), &FighterCharacter::take_damage);
	ClassDB::bind_method(D_METHOD("reset_stats"), &FighterCharacter::reset_stats);
}

void FighterCharacter::_ready() {
	nodes.facing = Object::cast_to<Node2D>(get_node("facing_container"));
	nodes.anim = Object::cast_to<AnimationPlayer>(get_node("facing_container/AnimationPlayer"));

	nodes.hitboxes[0] = Object::cast_to<Area2D>(get_node("facing_container/hitbox_punch"));
	nodes.hitboxes[1] = Object::cast_to<Area2D>(get_node("facing_container/hitbox_kick"));

	nodes.hurtboxes[0] = Object::cast_to<Area2D>(get_node("facing_container/hurtbox_standing"));
	nodes.hurtboxes[1] = Object::cast_to<Area2D>(get_node("facing_container/hurtbox_crouching"));

	nodes.counter_timer = memnew(Timer);
	nodes.counter_timer->set_one_shot(true);
	add_child(nodes.counter_timer);

	// Disable all hitboxes at start
	for (int i = 0; i < 2; i++) {
		if (nodes.hitboxes[i]) {
			nodes.hitboxes[i]->set_deferred("monitoring", false);
		}
	}
}

void FighterCharacter::_physics_process(double delta) {
	process_state(delta);
	process_buffer(delta);
	check_combos();
	process_animation();
}


void FighterCharacter::process_state(double delta) {
	if (combat.knocked_down) {
		timers.knockdown -= delta;
		if (timers.knockdown <= 0.0) {
			combat.knocked_down = false;
			safe_play("p1_get_up");
		}
		return;
	}

	if (combat.stunned) {
		timers.stun -= delta;
		if (timers.stun <= 0.0) {
			combat.stunned = false;
		}
		return;
	}

	process_movement(delta);
	process_combat(delta);
	process_blocking();
}


void FighterCharacter::process_movement(double delta) {
	apply_gravity(delta);

	double direction =
		Input::get_singleton()->get_action_strength("p1_walk_right") -
		Input::get_singleton()->get_action_strength("p1_walk_left");

	if (direction != 0 && nodes.facing) {
		nodes.facing->set_scale(Vector2(Math::sign(direction), 1));
	}

	if (movement.crouching) {
		velocity.x = 0;
	} else {
		velocity.x = direction * 800;
	}

	move_and_slide();
}

void FighterCharacter::apply_gravity(double delta) {
	if (!is_on_floor()) {
		velocity.y += 2200 * delta;
		movement.airborne = true;
	} else {
		velocity.y = 0;
		movement.airborne = false;
	}
}


void FighterCharacter::process_combat(double) {
	if (!can_accept_input()) return;

	if (Input::get_singleton()->is_action_just_pressed("p1_attack_j_simple")) {
		start_attack("punch");
	}
	else if (Input::get_singleton()->is_action_just_pressed("p1_attack_k_simple")) {
		start_attack("kick");
	}
}

bool FighterCharacter::can_accept_input() const {
	if (combat.attacking) return false;
	if (combat.stunned) return false;
	if (combat.knocked_down) return false;
	return true;
}

void FighterCharacter::start_attack(const String &id) {
	combat.attacking = true;

	if (id == "punch") {
		safe_play("p1_attack_j_simple");
		toggle_hitboxes(true, false);
	}
	else if (id == "kick") {
		safe_play("p1_attack_k_simple");
		toggle_hitboxes(false, true);
	}
}

void FighterCharacter::stop_attack() {
	combat.attacking = false;
	toggle_hitboxes(false, false);
}


void FighterCharacter::toggle_hitboxes(bool punch, bool kick) {
	bool flags[2] = { punch, kick };

	for (int i = 0; i < 2; i++) {
		if (nodes.hitboxes[i]) {
			nodes.hitboxes[i]->set_deferred("monitoring", flags[i]);
		}
	}
}


bool FighterCharacter::match_combo_recursive(
	const Array &combo,
	int combo_i,
	int buffer_i
) {
	if (combo_i < 0) return true;
	if (buffer_i < 0) return false;

	if (combo[combo_i] == input_buffer[buffer_i]) {
		return match_combo_recursive(combo, combo_i - 1, buffer_i - 1);
	}

	return false;
}

void FighterCharacter::check_combos() {
	Array keys = combos.keys();

	for (int i = 0; i < keys.size(); i++) {
		Array seq = combos[keys[i]];
		if (match_combo_recursive(seq, seq.size() - 1, input_buffer.size() - 1)) {
			safe_play("combo_punch_punch_kick");
			input_buffer.clear();
			combat.attacking = true;
			return;
		}
	}
}


void FighterCharacter::process_buffer(double delta) {
	if (Input::get_singleton()->is_action_just_pressed("p1_attack_j_simple"))
		input_buffer.append("p1_attack_j_simple");

	if (Input::get_singleton()->is_action_just_pressed("p1_attack_k_simple"))
		input_buffer.append("p1_attack_k_simple");

	if (input_buffer.size() > 10)
		input_buffer.remove(0);
}


void FighterCharacter::take_damage(int amount, Vector2 hit_pos) {
	if (combat.knocked_down) return;

	int final_damage = amount;

	if (combat.blocking) {
		final_damage *= 0.2;
		combat.counter_window = true;
		nodes.counter_timer->start(0.2);
	}

	vitals.health -= final_damage;

	if (vitals.health <= 0) {
		vitals.health = 0;
		safe_play("p1_defeat");
		return;
	}

	apply_knockback(hit_pos, 700);
	apply_stun(0.25);
}

void FighterCharacter::apply_knockback(Vector2 hit_pos, double force) {
	double dir = Math::sign(global_position.x - hit_pos.x);
	velocity.x = dir * force;
	velocity.y = -150;
}

void FighterCharacter::apply_stun(double duration) {
	combat.stunned = true;
	timers.stun = duration;
}


void FighterCharacter::process_animation() {
	if (!nodes.anim) return;

	if (combat.attacking || combat.stunned || combat.knocked_down)
		return;

	if (movement.airborne) {
		safe_play("p1_jump");
	}
	else if (movement.crouching) {
		safe_play("p1_crouch_idle");
	}
	else if (Math::abs(velocity.x) > 0) {
		safe_play("p1_walk");
	}
	else {
		safe_play("p1_idle");
	}
}

void FighterCharacter::safe_play(const String &anim) {
	if (nodes.anim && nodes.anim->has_animation(anim)) {
		nodes.anim->play(anim);
	}
}


void FighterCharacter::reset_stats() {
	vitals.health = vitals.max_health;
	combat = CombatState();
	timers = Timers();
	velocity = Vector2(0, 0);
	safe_play("p1_idle");
}
