#include "Player2.h"

using namespace godot;

Player2::Player2() {}
Player2::~Player2() {}

void Player2::_register_methods() {
	register_method("_ready", &Player2::_ready);
	register_method("_physics_process", &Player2::_physics_process);
	register_method("_on_animation_finished", &Player2::_on_animation_finished);
	register_method("_on_hurtbox_standing_area_entered", &Player2::_on_hurtbox_standing_area_entered);
	register_method("_on_hurtbox_crouching_area_entered", &Player2::_on_hurtbox_crouching_area_entered);

	register_property<Player2, String>("character_name", &Player2::character_name, "Player_2");
	register_property<Player2, int>("max_health", &Player2::max_health, 100);
	register_property<Player2, float>("hitstun_duration", &Player2::hitstun_duration, 0.25);
	register_property<Player2, float>("knockdown_duration", &Player2::knockdown_duration, 1.0);
	register_property<Player2, int>("damage_amount", &Player2::damage_amount, 10);
}

void Player2::_init() {
	health = max_health;
	is_attacking = false;
	current_attack = "";
	is_crouching = false;
	crouch_state = "none";
	is_stunned = false;
	stun_timer = 0;
	is_knocked_down = false;
	knockdown_time = 0;
	is_blocking = false;
	is_counter_window_active = false;

	combos = Dictionary();
	combos["punch_punch_kick"] = Array({"punch","punch","kick"});
	combos["kick_punch"] = Array({"kick","punch"});
}

void Player2::_ready() {
	facing_container = cast_to<Node2D>(get_node("facing_container"));
	animated_sprite = cast_to<AnimatedSprite2D>(facing_container->get_node("AnimatedSprite2D"));

	hurtbox_standing_area = cast_to<Hurtbox>(facing_container->get_node("hurtbox_standing"));
	hurtbox_crouching_area = cast_to<Hurtbox>(facing_container->get_node("hurtbox_crouching"));

	hitbox_punch_area = cast_to<Hitbox>(facing_container->get_node("hitbox_punch"));
	hitbox_kick_area = cast_to<Hitbox>(facing_container->get_node("hitbox_kick"));

	hitbox_punch_area->get_node("CollisionShape2D")->set_disabled(true);
	hitbox_kick_area->get_node("CollisionShape2D")->set_disabled(true);

	counter_window_timer = Timer::_new();
	add_child(counter_window_timer);
	counter_window_timer->set_one_shot(true);
	counter_window_timer->connect("timeout", Callable(this, "reset_stats"));
}

void Player2::_physics_process(float delta) {
	// ---------- STUN ----------
	if(is_stunned && !is_knocked_down) {
		_handle_stun(delta);
		return;
	}

	// ---------- KNOCKDOWN ----------
	if(is_knocked_down) {
		_handle_knockdown(delta);
		return;
	}

	// ---------- ATTACK ----------
	if(is_attacking) {
		_handle_attack_state(delta);
		return;
	}

	// ---------- NORMAL ----------
	bool crouch_pressed = Input::get_singleton()->is_action_pressed("p2_crouch");

	_handle_crouch(crouch_pressed);
	_handle_movement(delta);
	_handle_attack_input();
	_handle_animation();

	_handle_input_buffer(delta);
	check_for_combos();
	_handle_block();

	// Counter Attack
	if(is_counter_window_active && Input::get_singleton()->is_action_just_pressed("p2_attack_4_simple")) {
		if(is_crouching)
			_start_attack("counter_crouch_4");
		else
			_start_attack("counter_standing_4");

		is_counter_window_active = false;
		counter_window_timer->stop();
	}
}

// -------------------- STUN / KNOCKDOWN --------------------
void Player2::_handle_stun(float delta) {
	stun_timer -= delta;
	velocity.x = move_toward(velocity.x, 0, 400 * delta);
	move_and_slide();

	if(stun_timer <= 0) {
		is_stunned = false;
		animated_sprite->play("p2_idle");
	}
}

void Player2::_handle_knockdown(float delta) {
	knockdown_time -= delta;
	velocity.x = move_toward(velocity.x, 0, 50);
	velocity.y += get_gravity().y * delta;
	move_and_slide();

	if(knockdown_time <= 0) {
		is_knocked_down = false;
		animated_sprite->play("p2_knockdown_get_up");
	}
}

// -------------------- ATTACK STATE --------------------
void Player2::_handle_attack_state(float delta) {
	velocity.x = 0;
	if(!is_on_floor())
		velocity.y += get_gravity().y * delta;
	else
		velocity.y = 0;

	move_and_slide();
}

// -------------------- CROUCH SYSTEM --------------------
void Player2::_handle_crouch(bool crouch_pressed) {
	if(crouch_pressed && is_on_floor() && !is_attacking) {
		if(crouch_state == "none") {
			_start_crouch_down();
			return;
		}
		if(crouch_state == "idle") {
			animated_sprite->play("p2_crouch_idle");
			velocity.x = 0;
		}
	} else if(!crouch_pressed && is_crouching && !is_attacking) {
		_start_crouch_up();
		return;
	}

	hurtbox_standing_area->get_node("CollisionShape2D")->set_disabled(is_crouching);
	hurtbox_crouching_area->get_node("CollisionShape2D")->set_disabled(!is_crouching);
}

void Player2::_start_crouch_down() {
	crouch_state = "down";
	is_crouching = true;
	velocity.x = 0;
	animated_sprite->play("p2_crouch_down");

	hurtbox_standing_area->get_node("CollisionShape2D")->set_disabled(true);
	hurtbox_crouching_area->get_node("CollisionShape2D")->set_disabled(false);
}

void Player2::_start_crouch_up() {
	crouch_state = "up";
	animated_sprite->play("p2_crouch_up");

	hurtbox_standing_area->get_node("CollisionShape2D")->set_disabled(false);
	hurtbox_crouching_area->get_node("CollisionShape2D")->set_disabled(true);
}

// -------------------- MOVEMENT --------------------
void Player2::_handle_movement(float delta) {
	if(!is_on_floor()) {
		if(velocity.y > 0) velocity.y += get_gravity().y * 3 * delta;
		else velocity.y += get_gravity().y * delta;
	}

	// Jump
	if(Input::get_singleton()->is_action_just_pressed("p2_jump") && is_on_floor())
		velocity.y = JUMP_VELOCITY;

	// Walk
	float direction = Input::get_singleton()->get_action_strength("p2_walk_right") -
					  Input::get_singleton()->get_action_strength("p2_walk_left");

	if(direction != 0)
		facing_container->set_scale(Vector2(sign(direction), 1));

	if(is_crouching) velocity.x = 0;
	else velocity.x = direction * SPEED;

	move_and_slide();
}

// -------------------- ATTACK / COMBO SYSTEM --------------------
void Player2::_handle_attack_input() {
	if(is_attacking) return;

	float direction = Input::get_singleton()->get_action_strength("p2_walk_right") -
					  Input::get_singleton()->get_action_strength("p2_walk_left");

	if(Input::get_singleton()->is_action_just_pressed("p2_attack_4_simple")) {
		if(is_crouching) _start_attack(direction != 0 ? "crouch_side_4" : "crouch_4");
		else if(!is_on_floor()) _start_attack(direction != 0 ? "jump_side_4" : "jump_4");
		else _start_attack(direction != 0 ? "simple_side_4" : "simple_4");
	}

	if(Input::get_singleton()->is_action_just_pressed("p2_attack_5_simple")) {
		if(is_crouching) _start_attack(direction != 0 ? "crouch_side_5" : "crouch_5");
		else if(!is_on_floor()) _start_attack(direction != 0 ? "jump_side_5" : "jump_5");
		else _start_attack(direction != 0 ? "simple_side_5" : "simple_5");
	}
}

// -------------------- ANIMATION --------------------
void Player2::_handle_animation() {
	if(is_attacking || is_stunned) return;

	if(is_on_floor()) {
		if(crouch_state == "idle") animated_sprite->play("p2_crouch_idle");
		else if(crouch_state == "none") {
			float direction = Input::get_singleton()->get_action_strength("p2_walk_right") -
							  Input::get_singleton()->get_action_strength("p2_walk_left");
			animated_sprite->play(direction == 0 ? "p2_idle" : "p2_walk");
		}
	} else animated_sprite->play("p2_jump");
}

// -------------------- ATTACK START --------------------
void Player2::_start_attack(String type) {
	is_attacking = true;

	if(type == "crouch_4") current_attack = "p2_attack_crouch_4_simple";
	else if(type == "crouch_side_4") current_attack = "p2_attack_crouch_4_side";
	else if(type == "jump_4") current_attack = "p2_attack_jump_4_simple";
	else if(type == "jump_side_4") current_attack = "p2_attack_jump_4_side";
	else if(type == "simple_side_4") current_attack = "p2_attack_4_side";
	else if(type == "simple_4") current_attack = "p2_attack_4_simple";
	else if(type == "counter_standing_4") current_attack = "p2_standing_block_counter";
	else if(type == "counter_crouch_4") current_attack = "p2_crouch_block_counter";
	else if(type == "crouch_5") current_attack = "p2_attack_crouch_5_simple";
	else if(type == "crouch_side_5") current_attack = "p2_attack_crouch_5_side";
	else if(type == "jump_5") current_attack = "p2_attack_jump_5_simple";
	else if(type == "jump_side_5") current_attack = "p2_attack_jump_5_side";
	else if(type == "simple_side_5") current_attack = "p2_attack_5_side";
	else if(type == "simple_5") current_attack = "p2_attack_5_simple";

	if(type.find("4") != -1)
		hitbox_punch_area->get_node("CollisionShape2D")->set_disabled(false);
	if(type.find("5") != -1)
		hitbox_kick_area->get_node("CollisionShape2D")->set_disabled(false);

	animated_sprite->play(current_attack);
}

// -------------------- DAMAGE SYSTEM --------------------
void Player2::take_damage(int amount, Vector2 hit_position) {
	if(is_knocked_down) return;

	int final_damage = amount;
	float knockback_force = 900.0;
	bool play_get_hit = true;
	bool apply_stun = true;

	if(is_blocking) {
		final_damage = int(amount * 0.2);
		play_get_hit = false;
		apply_stun = false;

		is_counter_window_active = true;
		counter_window_timer->start(0.2);
	}

	health -= final_damage;
	health = Math::clamp(health, 0, max_health);

	// Knockback
	float dir = Math::sign(global_position.x - hit_position.x);
	velocity.x = dir * knockback_force;
	velocity.y = -10;

	if(final_damage >= 25) {
		is_knocked_down = true;
		knockdown_time = knockdown_duration;
		animated_sprite->play("p2_knockdown");
		return;
	}

	if(apply_stun) {
		is_stunned = true;
		stun_timer = hitstun_duration;
		is_attacking = false;

		if(play_get_hit) animated_sprite->play(is_crouching ? "p2_get_hit_crouch" : "p2_get_hit");
	} else animated_sprite->play(is_crouching ? "p2_block_crouch" : "p2_block_standing");

	if(health <= 0) {
		animated_sprite->play("p2_defeat");
		die();
	}
}

void Player2::die() {
	Godot::print("PLAYER DEAD");
}

// -------------------- BLOCK --------------------
void Player2::_handle_block() {
	bool block_pressed = Input::get_singleton()->is_action_pressed("p2_block_standing");

	if(block_pressed && is_on_floor() && !is_attacking) {
		if(!is_blocking) {
			is_blocking = true;
			animated_sprite->play(is_crouching ? "p2_block_crouch_start" : "p2_block_standing_start");
		} else animated_sprite->play(is_crouching ? "p2_block_crouch_idle" : "p2_block_standing_idle");
	} else if(is_blocking) {
		is_blocking = false;
		animated_sprite->play(is_crouching ? "p2_block_crouch_release" : "p2_block_standing_release");
	}
}

// -------------------- INPUT BUFFER --------------------
void Player2::_handle_input_buffer(float delta) {
	buffer_timer += delta;

	if(Input::get_singleton()->is_action_just_pressed("p2_attack_4_simple")) add_input_to_buffer("p2_attack_4_simple");
	if(Input::get_singleton()->is_action_just_pressed("p2_jump")) add_input_to_buffer("p2_jump");

	if(buffer_timer >= BUFFER_DURATION && !input_buffer.empty()) {
		input_buffer.pop_front();
		buffer_timer = 0.0;
	}
}

void Player2::add_input_to_buffer(String action) {
	input_buffer.append(action);
	if(input_buffer.size() > BUFFER_SIZE) input_buffer.pop_front();
	buffer_timer = 0.0;
}

// -------------------- COMBOS --------------------
void Player2::check_for_combos() {
	if(is_attacking || is_stunned) return;

	Array keys = combos.keys();
	for(int i=0; i<keys.size(); i++) {
		String combo_name = keys[i];
		Array seq = combos[combo_name];

		if(input_buffer.size() >= seq.size()) {
			Array slice = input_buffer.slice(input_buffer.size()-seq.size(), input_buffer.size());
			if(slice == seq) {
				perform_combo(combo_name);
				input_buffer.clear();
				return;
			}
		}
	}
}

void Player2::perform_combo(String combo_name) {
	if(combo_name == "punch_punch_kick") animated_sprite->play("combo_punch_punch_kick");
	else if(combo_name == "kick_punch") animated_sprite->play("combo_kick_punch");

	is_attacking = true;
	animated_sprite->connect("
