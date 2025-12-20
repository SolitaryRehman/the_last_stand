#include "Player2.h"

using namespace godot;

Player2::Player2() {}
Player2::~Player2() {}


void Player2::_register_methods() {
	register_method("_ready", &Player2::_ready);
	register_method("_physics_process", &Player2::_physics_process);
	register_method("take_damage", &Player2::take_damage);
	register_method("_on_animation_finished", &Player2::_on_animation_finished);

	register_property<Player2, String>("character_name", &Player2::character_name, "Player_2");
	register_property<Player2, int>("max_health", &Player2::max_health, 100);
	register_property<Player2, float>("hitstun_duration", &Player2::hitstun_duration, 0.25);
	register_property<Player2, float>("knockdown_duration", &Player2::knockdown_duration, 1.0);
}


void Player2::_init() {
	health = max_health;

	is_attacking = false;
	is_stunned = false;
	is_knocked_down = false;
	is_crouching = false;
	is_blocking = false;
	is_counter_window_active = false;

	stun_timer = 0;
	knockdown_time = 0;
	buffer_timer = 0;

	crouch_state = "none";
	current_attack = "";

	combos.clear();
	combos["punch_punch_kick"] = Array({"p2_attack_4", "p2_attack_4", "p2_attack_5"});
	combos["kick_punch"] = Array({"p2_attack_5", "p2_attack_4"});
}


void Player2::_ready() {
	facing_container = cast_to<Node2D>(get_node("facing_container"));
	animated_sprite = cast_to<AnimatedSprite2D>(facing_container->get_node("AnimatedSprite2D"));

	hurtbox_standing = cast_to<Hurtbox>(facing_container->get_node("hurtbox_standing"));
	hurtbox_crouching = cast_to<Hurtbox>(facing_container->get_node("hurtbox_crouching"));

	hitbox_punch = cast_to<Hitbox>(facing_container->get_node("hitbox_punch"));
	hitbox_kick = cast_to<Hitbox>(facing_container->get_node("hitbox_kick"));

	hitbox_punch->disable();
	hitbox_kick->disable();

	counter_timer = Timer::_new();
	counter_timer->set_one_shot(true);
	add_child(counter_timer);
	counter_timer->connect("timeout", Callable(this, "reset_counter"));
}


void Player2::_physics_process(float delta) {

	if (is_knocked_down) {
		handle_knockdown(delta);
		return;
	}

	if (is_stunned) {
		handle_stun(delta);
		return;
	}

	if (is_attacking) {
		apply_gravity(delta);
		move_and_slide();
		return;
	}

	handle_crouch();
	handle_movement(delta);
	handle_block();
	handle_attack_input();
	handle_animation();
	handle_input_buffer(delta);
	check_for_combos();

	// Counter
	if (is_counter_window_active &&
		Input::get_singleton()->is_action_just_pressed("p2_attack_4_simple")) {

		start_attack(is_crouching ? "counter_crouch" : "counter_stand");
		reset_counter();
	}
}


void Player2::handle_movement(float delta) {
	apply_gravity(delta);

	float dir =
		Input::get_singleton()->get_action_strength("p2_walk_right") -
		Input::get_singleton()->get_action_strength("p2_walk_left");

	if (dir != 0)
		facing_container->set_scale(Vector2(Math::sign(dir), 1));

	if (!is_crouching)
		velocity.x = dir * SPEED;
	else
		velocity.x = 0;

	if (Input::get_singleton()->is_action_just_pressed("p2_jump") && is_on_floor())
		velocity.y = JUMP_VELOCITY;

	move_and_slide();
}

void Player2::apply_gravity(float delta) {
	if (!is_on_floor())
		velocity.y += get_gravity().y * delta;
}


void Player2::handle_crouch() {
	bool crouch = Input::get_singleton()->is_action_pressed("p2_crouch");

	if (crouch && is_on_floor() && !is_attacking) {
		is_crouching = true;
		crouch_state = "idle";
	} else if (!crouch) {
		is_crouching = false;
		crouch_state = "none";
	}

	hurtbox_standing->set_disabled(is_crouching);
	hurtbox_crouching->set_disabled(!is_crouching);
}


void Player2::handle_attack_input() {
	if (is_attacking) return;

	if (Input::get_singleton()->is_action_just_pressed("p2_attack_4_simple"))
		start_attack("punch");

	if (Input::get_singleton()->is_action_just_pressed("p2_attack_5_simple"))
		start_attack("kick");
}

void Player2::start_attack(String type) {
	is_attacking = true;

	if (type == "punch") {
		current_attack = "p2_attack_4_simple";
		hitbox_punch->enable();
	}
	else if (type == "kick") {
		current_attack = "p2_attack_5_simple";
		hitbox_kick->enable();
	}
	else if (type == "counter_stand")
		current_attack = "p2_block_counter";
	else if (type == "counter_crouch")
		current_attack = "p2_crouch_block_counter";

	animated_sprite->play(current_attack);
}


void Player2::take_damage(int amount, Vector2 hit_pos) {
	if (is_knocked_down) return;

	int dmg = amount;

	if (is_blocking) {
		dmg *= 0.2;
		is_counter_window_active = true;
		counter_timer->start(0.2);
	}

	health -= dmg;
	health = Math::clamp(health, 0, max_health);

	velocity.x = Math::sign(global_position.x - hit_pos.x) * 800;
	velocity.y = -200;

	if (dmg >= 25) {
		is_knocked_down = true;
		knockdown_time = knockdown_duration;
		animated_sprite->play("p2_knockdown");
		return;
	}

	is_stunned = true;
	stun_timer = hitstun_duration;
	animated_sprite->play(is_crouching ? "p2_get_hit_crouch" : "p2_get_hit");

	if (health <= 0)
		animated_sprite->play("p2_defeat");
}


void Player2::handle_stun(float delta) {
	stun_timer -= delta;
	velocity.x = move_toward(velocity.x, 0, 400 * delta);
	move_and_slide();

	if (stun_timer <= 0) {
		is_stunned = false;
		animated_sprite->play("p2_idle");
	}
}

void Player2::handle_knockdown(float delta) {
	knockdown_time -= delta;
	apply_gravity(delta);
	move_and_slide();

	if (knockdown_time <= 0) {
		is_knocked_down = false;
		animated_sprite->play("p2_knockdown_get_up");
	}
}


void Player2::handle_block() {
	bool block = Input::get_singleton()->is_action_pressed("p2_block_standing");

	if (block && !is_attacking && is_on_floor()) {
		is_blocking = true;
		animated_sprite->play(is_crouching ? "p2_block_crouch_idle" : "p2_block_standing_idle");
	} else {
		is_blocking = false;
	}
}


void Player2::handle_input_buffer(float delta) {
	buffer_timer += delta;

	if (Input::get_singleton()->is_action_just_pressed("p2_attack_4_simple"))
		input_buffer.append("p2_attack_4");
	if (Input::get_singleton()->is_action_just_pressed("p2_attack_5_simple"))
		input_buffer.append("p2_attack_5");

	if (input_buffer.size() > BUFFER_SIZE)
		input_buffer.pop_front();

	if (buffer_timer > BUFFER_DURATION)
		input_buffer.clear();
}

void Player2::check_for_combos() {
	if (is_attacking) return;

	Array keys = combos.keys();
	for (int i = 0; i < keys.size(); i++) {
		String key = keys[i];
		Array seq = combos[key];

		if (input_buffer.size() >= seq.size()) {
			Array slice = input_buffer.slice(input_buffer.size() - seq.size(), input_buffer.size());
			if (slice == seq) {
				animated_sprite->play("p2_combo_" + key);
				is_attacking = true;
				input_buffer.clear();
				return;
			}
		}
	}
}


void Player2::reset_counter() {
	is_counter_window_active = false;
}

void Player2::_on_animation_finished() {
	is_attacking = false;
	hitbox_punch->disable();
	hitbox_kick->disable();
}
