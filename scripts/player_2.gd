extends CharacterBody2D

# ============================================================
# CONSTANTS
# ============================================================
const SPEED = 400.0
const JUMP_VELOCITY = -650.0

const BUFFER_SIZE = 10
const BUFFER_DURATION = 0.3

# ============================================================
# ONREADY NODES
# ============================================================
@onready var facing_container: Node2D = $facing_container
@onready var animated_sprite: AnimatedSprite2D = $facing_container/AnimatedSprite2D

# Hurtboxes
@onready var hurtbox_standing_area: Hurtbox = $facing_container/hurtbox_standing
@onready var hurtbox_crouching_area: Hurtbox = $facing_container/hurtbox_crouching

# Hitboxes
@onready var hitbox_punch_area: Hitbox = $facing_container/hitbox_punch
@onready var hitbox_kick_area: Hitbox = $facing_container/hitbox_kick

# ============================================================
# SIGNAL / EXPORT
# ============================================================
@export var character_name: String = "Player_2"
@export var max_health: int = 100
@export var hitstun_duration := 0.25
@export var knockdown_duration := 1.0
@export var damage_amount := 10

signal character_died(character_id: String)
signal health_changed(new_health: int, character_id: String)

# ============================================================
# STATE VARIABLES
# ============================================================
var health: int = max_health

# Attacking
var current_attack = ""
var is_attacking = false

# Crouch
var is_crouching = false
var crouch_state = "none"   # none / down / idle / up

# Stun / Knockdown
var is_stunned: bool = false
var stun_timer: float = 0.0
var is_knocked_down: bool = false
var knockdown_time: float = 0.0

# Blocking
var is_blocking: bool = false
var is_blocking_hit = false
var is_counter_window_active: bool = false

# Input buffer
var input_buffer: Array[String] = []
var buffer_timer: float = 0.0

# Counter Timer
var counter_window_timer: Timer = Timer.new()

# ============================================================
# COMBO DEFINITIONS
# ============================================================
const COMBOS = {
	"punch_punch_kick": ["punch", "punch", "kick"],
	"kick_punch": ["kick", "punch"]
}

# ============================================================
# READY
# ============================================================
func _ready():
	animated_sprite.animation_finished.connect(_on_animation_finished)

	# Disable hitboxes by default
	hitbox_punch_area.get_node("CollisionShape2D").disabled = true
	hitbox_kick_area.get_node("CollisionShape2D").disabled = true

	add_child(counter_window_timer)
	counter_window_timer.one_shot = true
	counter_window_timer.timeout.connect(func(): is_counter_window_active = false)

# ============================================================
# MAIN PHYSICS PROCESS
# ============================================================
func _physics_process(delta: float) -> void:

	# -------- STUN --------
	if is_stunned and not is_knocked_down:
		_handle_stun(delta)
		return

	# -------- KNOCKDOWN --------
	if is_knocked_down:
		_handle_knockdown(delta)
		return

	# -------- ATTACK LOCK --------
	if is_attacking:
		_handle_attack_state(delta)
		return

	# -------- NORMAL STATE --------
	var crouch_pressed := Input.is_action_pressed("p2_crouch")

	_handle_crouch(crouch_pressed)
	_handle_movement(delta)
	_handle_attack_input()
	_handle_animation()

	# -------- INPUT BUFFER --------
	_handle_input_buffer(delta)
	check_for_combos()

	# -------- BLOCK --------
	_handle_block()

	# -------- COUNTER ATTACK --------
	if is_counter_window_active and Input.is_action_just_pressed("p2_attack_4_simple"):
		
		if is_crouching:
			_start_attack("counter_crouch_4")
		else:
			_start_attack("counter_standing_4")

		is_counter_window_active = false
		counter_window_timer.stop()

# ============================================================
# STUN / KNOCKDOWN HANDLERS
# ============================================================
func _handle_stun(delta):
	
	stun_timer -= delta
	velocity.x = move_toward(velocity.x, 0, 400 * delta)
	move_and_slide()

	if stun_timer <= 0:
		is_stunned = false
		animated_sprite.play("p2_idle")

func _handle_knockdown(delta):
	
	knockdown_time -= delta
	velocity.x = move_toward(velocity.x, 0, 50)
	velocity.y += get_gravity().y * delta

	move_and_slide()

	if knockdown_time <= 0:
		is_knocked_down = false
		animated_sprite.play("p2_knockdown_get_up")

# ============================================================
# ATTACK STATE
# ============================================================
func _handle_attack_state(delta):
	velocity.x = 0
	
	if not is_on_floor():
		velocity.y += get_gravity().y * delta
	else:
		velocity.y = 0
	
	move_and_slide()

# ============================================================
# CROUCH SYSTEM
# ============================================================
func _handle_crouch(crouch_pressed):
	
	# Start / Hold Crouch
	if crouch_pressed and is_on_floor() and not is_attacking:

		if crouch_state == "none":
			_start_crouch_down()
			return
		
		if crouch_state == "idle":
			animated_sprite.play("p2_crouch_idle")
			velocity.x = 0

	# Stop Crouch
	elif not crouch_pressed and is_crouching and not is_attacking:
		_start_crouch_up()
		return

	# Manage hurtboxes
	hurtbox_standing_area.get_node("CollisionShape2D").disabled = is_crouching
	hurtbox_crouching_area.get_node("CollisionShape2D").disabled = not is_crouching

func _start_crouch_down():
	
	crouch_state = "down"
	is_crouching = true
	velocity.x = 0
	animated_sprite.play("p2_crouch_down")
	
	hurtbox_standing_area.get_node("CollisionShape2D").disabled = true
	hurtbox_crouching_area.get_node("CollisionShape2D").disabled = false

func _start_crouch_up():
	
	crouch_state = "up"
	animated_sprite.play("p2_crouch_up")
	
	hurtbox_standing_area.get_node("CollisionShape2D").disabled = false
	hurtbox_crouching_area.get_node("CollisionShape2D").disabled = true

# ============================================================
# MOVEMENT SYSTEM
# ============================================================
func _handle_movement(delta):
	
	if not is_on_floor():
		velocity += get_gravity() * delta

	# Jump
	if Input.is_action_just_pressed("p2_jump") and is_on_floor():
		velocity.y = JUMP_VELOCITY

	# Walk
	var direction := Input.get_axis("p2_walk_left", "p2_walk_right")

	# Flip
	if direction != 0:
		facing_container.scale.x = sign(direction)

	# Apply
	if is_crouching:
		velocity.x = 0
		
	else:
		if direction != 0:
			velocity.x = direction * SPEED
		else:
			velocity.x = move_toward(velocity.x, 0, SPEED)

	move_and_slide()

# ============================================================
# ATTACK INPUT SYSTEM
# ============================================================
func _handle_attack_input():

	if is_attacking:
		return

	var direction := Input.get_axis("p2_walk_left", "p2_walk_right")

	# -------- ATTACK 4 --------
	if Input.is_action_just_pressed("p2_attack_4_simple"):

		if is_crouching:
			if direction != 0:
				_start_attack("crouch_side_4")
			else:
				_start_attack("crouch_4")
			return

		if not is_on_floor():
			if direction != 0:
				_start_attack("jump_side_4")
			else:
				_start_attack("jump_4")
			return

		if direction != 0:
			_start_attack("simple_side_4")
			return

		_start_attack("simple_4")

	# -------- ATTACK 5 --------
	if Input.is_action_just_pressed("p2_attack_5_simple"):

		if is_crouching:
			if direction != 0:
				_start_attack("crouch_side_5")
			else:
				_start_attack("crouch_5")
			return

		if not is_on_floor():
			if direction != 0:
				_start_attack("jump_side_5")
			else:
				_start_attack("jump_5")
			return

		if direction != 0:
			_start_attack("simple_side_5")
			return

		_start_attack("simple_5")

# ============================================================
# ANIMATION LOGIC
# ============================================================
func _handle_animation():

	if is_attacking or is_stunned:
		return

	if is_on_floor():

		if crouch_state == "idle":
			animated_sprite.play("p2_crouch_idle")
			return

		if crouch_state in ["up", "down"]:
			return

		var direction := Input.get_axis("p2_walk_left", "p2_walk_right")

		if direction == 0:
			animated_sprite.play("p2_idle")
		else:
			animated_sprite.play("p2_walk")

	else:
		animated_sprite.play("p2_jump")

# ============================================================
# ANIMATION FINISHED
# ============================================================
func _on_animation_finished():

	if animated_sprite.animation == current_attack:
		is_attacking = false
		current_attack = ""

		hitbox_punch_area.get_node("CollisionShape2D").disabled = true
		hitbox_kick_area.get_node("CollisionShape2D").disabled = true

		if is_crouching and not Input.is_action_pressed("p2_crouch"):
			_start_crouch_up()
		return

	if animated_sprite.animation == "p2_crouch_down":
		crouch_state = "idle"
		animated_sprite.play("p2_crouch_idle")
		return

	if animated_sprite.animation == "p2_crouch_up":
		crouch_state = "none"
		is_crouching = false
		animated_sprite.play("p2_idle")
		return

	if animated_sprite.animation == "getup":
		animated_sprite.play("p2_idle")
		return

	if animated_sprite.animation == "p2_get_hit":
		if not is_knocked_down:
			animated_sprite.play("p2_idle")
		return

# ============================================================
# ATTACK START
# ============================================================
func _start_attack(type):

	is_attacking = true

	match type:
		"crouch_4":
			current_attack = "p2_attack_crouch_4_simple"
		"crouch_side_4":
			current_attack = "p2_attack_crouch_4_side"
		"jump_4":
			current_attack = "p2_attack_jump_4_simple"
		"jump_side_4":
			current_attack = "p2_attack_jump_4_side"
		"simple_side_4":
			current_attack = "p2_attack_4_side"
		"simple_4":
			current_attack = "p2_attack_4_simple"
		"counter_standing_4":


			current_attack = "p2_standing_block_counter"
		"counter_crouch_4": 
			current_attack = "p2_crouch_block_counter"


		"crouch_5":
			current_attack = "p2_attack_crouch_5_simple"
		"crouch_side_5":
			current_attack = "p2_attack_crouch_5_side"
		"jump_5":
			current_attack = "p2_attack_jump_5_simple"
		"jump_side_5":
			current_attack = "p2_attack_jump_5_side"
		"simple_side_5":
			current_attack = "p2_attack_5_side"
		"simple_5":
			current_attack = "p2_attack_5_simple"

	# Enable hitbox based on attack type
	if "4" in type:
		hitbox_punch_area.get_node("CollisionShape2D").disabled = false
	if "5" in type:
		hitbox_kick_area.get_node("CollisionShape2D").disabled = false

	animated_sprite.play(current_attack)

# ============================================================
# DAMAGE SYSTEM
# ============================================================
func take_damage(amount: int, hit_position: Vector2):

	if is_knocked_down:
		return

	var final_damage = amount
	var knockback_force = 400.0
	var knockback_reduction = 1.0
	var play_get_hit = true
	var apply_stun = true

	# -------- BLOCKING --------
	if is_blocking:
		final_damage = int(amount * 0.2)
		knockback_reduction = 0.1
		play_get_hit = false
		apply_stun = false

		is_counter_window_active = true
		counter_window_timer.start(0.2)

	# -------- APPLY DAMAGE --------
	health -= final_damage
	health = clamp(health, 0, max_health)

	health_changed.emit(health, character_name)

	# -------- KNOCKBACK --------
	var dir = sign(global_position.x - hit_position.x)
	velocity.x = dir * knockback_force
	velocity.y = -10

	# -------- KNOCKDOWN --------
	if final_damage >= 25:
		is_knocked_down = true
		knockdown_time = knockdown_duration
		animated_sprite.play("p2_knockdown")
		return

	# -------- STUN / HIT ANIM --------
	if apply_stun:
		is_stunned = true
		stun_timer = hitstun_duration
		is_attacking = false

		if play_get_hit:
			if is_crouching:
				animated_sprite.play("p2_get_hit_crouch")
			else:
				animated_sprite.play("p2_get_hit")


	else:
		if is_crouching:
			animated_sprite.play("p2_block_crouch")
		else:
			animated_sprite.play("p2_block_standing")


	# -------- DEATH --------
	if health <= 0:
		animated_sprite.play("p2_defeat")
		emit_signal("character_died", character_name)
		die()

func die():
	print("PLAYER DEAD")

# ============================================================
# HITBOX SIGNALS
# ============================================================
func _on_hurtbox_standing_area_entered(area: Area2D) -> void:
	if area.owner == self: 
		return
	
	if area.is_in_group("hitbox_punch"): 
		_take_punch_hit(area)
	
	if area.is_in_group("hitbox_kick"): 
		_take_kick_hit(area)

func _on_hurtbox_crouching_area_entered(area: Area2D) -> void:
	if area.owner == self: 
		return
	
	if area.is_in_group("hitbox_punch"): 
		_take_punch_hit(area)
	
	if area.is_in_group("hitbox_kick"): 
		_take_kick_hit(area)

# ============================================================
# APPLY HIT
# ============================================================
func _take_punch_hit(area: Area2D):
	
	take_damage(10, area.global_position)
	is_stunned = true
	stun_timer = hitstun_duration
	
	if is_crouching:
		animated_sprite.play("p2_get_hit_crouch")
	else:
		animated_sprite.play("p2_get_hit")


func _take_kick_hit(area: Area2D):
	
	take_damage(5, area.global_position)
	is_knocked_down = true
	knockdown_time = knockdown_duration

	velocity.y = -200
	velocity.x = sign(global_position.x - area.global_position.x) * 700
	animated_sprite.play("p2_knockdown")

# ============================================================
# INPUT BUFFER SYSTEM
# ============================================================
func _handle_input_buffer(delta):
	buffer_timer += delta

	if Input.is_action_just_pressed("p2_attack_4_simple"):
		add_input_to_buffer("p2_attack_4_simple")
	
	if Input.is_action_just_pressed("p2_jump"):
		add_input_to_buffer("p2_jump")

	if buffer_timer >= BUFFER_DURATION and not input_buffer.is_empty():
		input_buffer.pop_front()
		buffer_timer = 0.0

func add_input_to_buffer(action: String):
	input_buffer.append(action)
	
	if input_buffer.size() > BUFFER_SIZE:
		input_buffer.pop_front()
	
	buffer_timer = 0.0

# ============================================================
# COMBO SYSTEM
# ============================================================
func check_for_combos():
	if is_attacking or is_stunned:
		return

	for combo_name in COMBOS:
		var seq = COMBOS[combo_name]

		if input_buffer.size() >= seq.size():
			var slice = input_buffer.slice(input_buffer.size() - seq.size(), input_buffer.size())

			if slice == seq:
				perform_combo(combo_name)
				input_buffer.clear()
				return

func perform_combo(combo_name: String):

	match combo_name:
		"punch_punch_kick":
			animated_sprite.play("combo_punch_punch_kick")
		"kick_punch":
			animated_sprite.play("combo_kick_punch")

	is_attacking = true
	animated_sprite.animation_finished.connect(_on_animation_finished, CONNECT_ONE_SHOT)

	hitbox_punch_area.get_node("CollisionShape2D").disabled = false
	hitbox_kick_area.get_node("CollisionShape2D").disabled = false

# ============================================================
# BLOCK SYSTEM
# ============================================================
func _handle_block():

	if Input.is_action_pressed("p2_block_standing") and is_on_floor() and not is_attacking:

		if not is_blocking:
			is_blocking = true

			if is_crouching: animated_sprite.play("p2_block_crouch_start")
			else: animated_sprite.play("p2_block_standing_start")

		else:
			if is_crouching: animated_sprite.play("p2_block_crouch_idle")
			else: animated_sprite.play("p2_block_standing_idle")

	else:
		if is_blocking:
			is_blocking = false

			if is_crouching: animated_sprite.play("p2_block_crouch_release")
			else: animated_sprite.play("p2_block_standing_release")
	


func reset_stats():
	health = max_health
	is_stunned = false
	is_attacking = false
	is_blocking = false
	velocity = Vector2.ZERO
	health_changed.emit(health, character_name)
