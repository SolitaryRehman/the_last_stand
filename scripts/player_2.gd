extends CharacterBody2D

const SPEED = 400.0
const JUMP_VELOCITY = -650.0

@onready var animated_sprite: AnimatedSprite2D = $facing_container/AnimatedSprite2D
@onready var hurtbox_area: Hurtbox = $facing_container/hurtbox
@onready var hitbox_area: Hitbox = $facing_container/hitbox

@onready var facing_container: Node2D = $facing_container

@export var character_name: String = "Player_2"

signal character_died(character_id: String)
signal health_changed(new_health: int, character_id: String)

# ================================
# PLAYER STATE
# ================================
var current_attack = ""
var is_attacking = false

var is_crouching = false
var crouch_state = "none"  # "none", "down", "idle", "up"

@export var max_health: int = 100
var health: int = max_health

var is_stunned: bool = false   # Flag for stun mechanics
var stun_timer: float = 0.0 # Timer for stun duration

var is_knocked_down: bool = false
var knockdown_time: float = 0.0

@export var hitstun_duration := 0.25
@export var knockdown_duration := 1.0

@export var damage_amount = 10

# ---------- BUFFER STATE --------------- 
var input_buffer: Array[String] = []
const BUFFER_SIZE = 10 # Store up to 10 recent inputs
const BUFFER_DURATION = 0.3 # Inputs expire after 0.3 seconds
var buffer_timer: float = 0.0

var is_blocking: bool = false

var counter_window_timer: Timer = Timer.new() # Add as child node


# ====== DEFINING COMBO SEQUENCES ======
const COMBOS = {
		"punch_punch_kick": ["punch", "punch", "kick"],
		"kick_punch": ["kick", "punch"],
		# Will add more combos here later
	}


func _ready():
	animated_sprite.animation_finished.connect(_on_animation_finished)
	health = max_health
	
	# Ensure hitbox is disabled by default
	hitbox_area.get_node("CollisionShape2D").disabled = true
	
	add_child(counter_window_timer)
	counter_window_timer.one_shot = true
	counter_window_timer.timeout.connect(func(): is_counter_window_active = false)

var is_counter_window_active: bool = false

# ============================================================
# MAIN PHYSICS PROCESS
# ============================================================
func _physics_process(delta: float) -> void:
	
	# ================= HITSTUN ==================
	if is_stunned and not is_knocked_down:
	
		stun_timer -= delta
		
		# Gradually reduce horizontal knockback (friction)
		var decel = 400 * delta
		
		if velocity.x > 0:
			velocity.x = max(0, velocity.x - decel)
		elif velocity.x < 0:
			velocity.x = min(0, velocity.x + decel)
			
		move_and_slide()
		
		if stun_timer <= 0:
			is_stunned = false
			animated_sprite.play("p2_idle")
		
		return
	
	# ================= KNOCKDOWN ==================
	if is_knocked_down:
		knockdown_time -= delta
			
		velocity.x = move_toward(velocity.x, 0, 50)
		velocity.y += get_gravity().y * delta
	
		move_and_slide()
	
		if knockdown_time <= 0:
			is_knocked_down = false
			animated_sprite.play("getup")
	
		return


	if is_attacking:
		_handle_attack_state(delta)
		return
	
	var crouch_pressed := Input.is_action_pressed("p2_crouch")
	
	_handle_crouch(crouch_pressed)
	_handle_movement(delta)
	_handle_attack_input()
	_handle_animation()
	
	# ========= COMBO SYSTEM STUFF ==========
	# DOOOOOOOO ITTTTTTTTTTTTTT LATERRRRRRRRRRRRRRR IMPPPPPPPPPPPP
	
	# Add new inputs to buffer
	if Input.is_action_just_pressed("p2_attack_4_simple"):
		add_input_to_buffer("p2_attack_4_simple")
	if Input.is_action_just_pressed("p2_attack_4_simple"):
		add_input_to_buffer("p2_attack_4_simple")
	if Input.is_action_just_pressed("p2_jump"):
		add_input_to_buffer("p2_jump")
		# Add other actions '''''''''' LATER  '''''''''''''''''''''
	
	# To clear old inputs from buffer (simple time-based expiry)
	buffer_timer += delta
	
	if buffer_timer >= BUFFER_DURATION:
		
		if not input_buffer.is_empty():
			input_buffer.pop_front() # Remove oldest input
		
		buffer_timer = 0.0
	
		# Check for combos (call this before handling single inputs to prioritize combos)
	check_for_combos()
	
	# Handle stun timer
	if is_stunned:
		stun_timer -= delta
		if stun_timer <= 0:
			is_stunned = false
		else:
			# If stunned, prevent horizontal movement input from affecting velocity.x
			# Gravity still applies.
			
			velocity.x = 0 
			
			move_and_slide()
			_handle_animation()
			return # Exit early if stunned
	
		move_and_slide()
		_handle_animation()
	
	# Handle block input
	if Input.is_action_pressed("p2_block_standing") and not is_attacking and not is_stunned:
		is_blocking = true
		
		if is_crouching:
			animated_sprite.play("p2_block_crouch")
		else :
			animated_sprite.play("p2_block_standing")
	else:
		is_blocking = false # Ensure block state is reset when button is released
	
	# Check for counterattack input during counter window
	if is_counter_window_active and Input.is_action_just_pressed("p2_attack_4_simple"):
		
		print("Counterattack!")
		if is_crouching:
			_start_attack("counter_crouch") 
		else:
			_start_attack("counter_standing")
		
		is_counter_window_active = false
		counter_window_timer.stop()
		# Add counter-specific effects ''' LATER '''(e.g., higher damage, stun)


func add_input_to_buffer(action: String):
	
	input_buffer.append(action)
	
	if input_buffer.size() > BUFFER_SIZE:
		input_buffer.pop_front() # Keep buffer size limited
	
	buffer_timer = 0.0 # Reset timer on new input


func check_for_combos():
	if is_attacking or is_stunned: return # Cannot combo while attacking or stunned
	
	for combo_name in COMBOS:
		var combo_sequence = COMBOS[combo_name]
		
		if input_buffer.size() >= combo_sequence.size():
			
			# Check if the end of the buffer matches the combo sequence
			var buffer_slice = input_buffer.slice(input_buffer.size() - combo_sequence.size(), input_buffer.size())
			
			if buffer_slice == combo_sequence:
				
				print("Combo executed: ", combo_name)
				perform_combo(combo_name)
				input_buffer.clear() # Clear buffer after successful combo
				return # Only execute one combo at a time
	


func perform_combo(combo_name: String):
	match combo_name:
		"punch_punch_kick":
			
			# Play a specific combo animation or sequence of animations
			animated_sprite.play("combo_punch_punch_kick")
			# Enable/disable specific hitboxes for the combo
		"kick_punch":
			
			animated_sprite.play("combo_kick_punch")
			# ============ WILL ADD MORE COMBOS SOON ===============
		
	is_attacking = true # Set attacking flag for the duration of the combo
	animated_sprite.animation_finished.connect(_on_animation_finished, CONNECT_ONE_SHOT)
	hitbox_area.get_node("CollisionShape2D").disabled = false # Enable hitbox for combo
	

# ============================================================
# ATTACK STATE LOGIC
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

	# Start / Hold crouch
	if crouch_pressed and is_on_floor() and not is_attacking:

		if crouch_state == "none":
			_start_crouch_down()
			return

		if crouch_state == "down":
			return

		if crouch_state == "idle":
			animated_sprite.play("p2_crouch_idle")
			velocity.x = 0
			# don't return (allow crouch attacks)

	# Stop crouching
	elif not crouch_pressed and is_crouching and not is_attacking:
		_start_crouch_up()
		return


func _start_crouch_down():
	crouch_state = "down"
	is_crouching = true
	velocity.x = 0
	animated_sprite.play("p2_crouch_down")


func _start_crouch_up():
	crouch_state = "up"
	animated_sprite.play("p2_crouch_up")


# ============================================================
# MOVEMENT SYSTEM
# ============================================================
func _handle_movement(delta):

	# gravity
	if not is_on_floor():
		velocity += get_gravity() * delta

	# jump
	if Input.is_action_just_pressed("p2_jump") and is_on_floor():
		velocity.y = JUMP_VELOCITY

	# walking
	var direction := Input.get_axis("p2_walk_left", "p2_walk_right")

	# flip sprite
	if direction > 0:
		facing_container.scale.x = 1
	elif direction < 0:
		facing_container.scale.x = -1

	# apply movement
	if not is_crouching:  # can't walk while crouched
		if direction:
			velocity.x = direction * SPEED
		else:
			velocity.x = move_toward(velocity.x, 0, SPEED)
	else:
		velocity.x = 0

	move_and_slide()


# ============================================================
# ATTACK INPUT SYSTEM
# ============================================================
func _handle_attack_input():

	if not Input.is_action_just_pressed("p2_attack_4_simple"):
		return

	if is_attacking:
		return

	var direction := Input.get_axis("p2_walk_left", "p2_walk_right")

	# CROUCH ATTACKS
	if is_crouching:
		if direction != 0:
			_start_attack("crouch_side")
		else:
			_start_attack("crouch")
		return

	# AIR ATTACKS
	if not is_on_floor():
		if direction != 0:
			_start_attack("jump_side")
		else:
			_start_attack("jump")
		return

	# SIDE ATTACKS
	if direction != 0:
		_start_attack("simple_side")
		return

	# NEUTRAL ATTACK
	_start_attack("simple")
	

# ============================================================
# ANIMATION CONTROLLER
# ============================================================
func _handle_animation():

	if is_attacking:
		return

	if is_on_floor():

		if crouch_state == "idle":
			animated_sprite.play("p2_crouch_idle")
			return

		if crouch_state == "up" or crouch_state == "down":
			return

		var direction := Input.get_axis("p2_walk_left", "p2_walk_right")

		if direction == 0:
			animated_sprite.play("p2_idle")
		else:
			animated_sprite.play("p2_walk")
	else:
		animated_sprite.play("p2_jump")

# ============================================================
# ANIMATION FINISHED HANDLER
# ============================================================
func _on_animation_finished():

	# Attack finished
	if animated_sprite.animation == current_attack:
		is_attacking = false
		hitbox_area.get_node("CollisionShape2D").disabled = true # Disable hitbox
		current_attack = ""

		if is_crouching and not Input.is_action_pressed("p2_crouch"):
			_start_crouch_up()
		return

	# Crouch Down → Crouch Idle
	if animated_sprite.animation == "p2_crouch_down":
		crouch_state = "idle"
		animated_sprite.play("p2_crouch_idle")
		return

	# Crouch Up → Standing
	if animated_sprite.animation == "p2_crouch_up":
		crouch_state = "none"
		is_crouching = false
		animated_sprite.play("p2_idle")
		return
	
	# Finish knockdown getup
	if animated_sprite.animation == "getup":
		animated_sprite.play("p2_idle")
		return

	# Finish hit animation (not knockdown)
	if animated_sprite.animation == "p2_get_hit":
		if not is_knocked_down:
			animated_sprite.play("p2_idle")
		return



# ============================================================
# ATTACK START LOGIC
# ============================================================
func _start_attack(type):

	is_attacking = true

	match type:
		"crouch":
			current_attack = "p2_attack_crouch_4_simple"
		"crouch_side":
			current_attack = "p2_attack_crouch_4_side"
		"jump":
			current_attack = "p2_attack_jump_4_simple"
		"jump_side":
			current_attack = "p2_attack_jump_4_side"
		"simple_side":
			current_attack = "p2_attack_4_side"
		"simple":
			current_attack = "p2_attack_4_simple"
		"counter_standing":
			current_attack = "p2_standing_block_counter"
		"counter_crouch":
			current_attack = "p2_crouch_block_counter"
		
	animated_sprite.play(current_attack)
	
	hitbox_area.get_node("CollisionShape2D").disabled = false # enable hitbox


# ============================================================
# HIT + DAMAGE SYSTEM
# ============================================================

func take_damage(amount: int, hit_position: Vector2):
	
	if is_knocked_down: 
		return 
	
	var final_damage = amount
	var knockback_force = 400.0
	var knockback_reduction = 1.0     # default = full knockback
	var play_getting_hit_animation = true     
	var apply_stun = true
	
	if is_blocking:
		
		# Reduce damage by 80% when blocking
		final_damage = int(amount * 0.2) 
		
		# Knockback reduced to 10%
		knockback_reduction = 0.10
		
		# Do NOT play hit animation while blocking
		play_getting_hit_animation = false
		
		# Do NOT stun on block
		apply_stun = false
		
		# Open counter window
		is_counter_window_active = true
		counter_window_timer.start(0.2)

	# ========== APPLY DAMAGE ===========
	health -= final_damage
	
	# Ensure health doesn't go below 0 or above max_health
	health = clamp(health, 0, max_health)
	print("Character took ", amount, " damage. Health: ", health) # For debugging
	
	# ADDED an @export var character_name: String = "Player1" to Character.gd
	health_changed.emit(health, name)
	
	# HORIZONTAL KNOCKBACK OPPOSITE ATTACKER
	var knockback_direction_difference = global_position.x - hit_position.x
	var dir = 0
	
	if knockback_direction_difference > 0:
		dir = 1  # attacker left → push right
	else:
		dir = -1 # attacker right → push left
	
	velocity.x = dir * knockback_force
	velocity.y = -10       # very small vertical movement
	
	print("Hit from ", hit_position.x, " | Player pos ", global_position.x, " | Diff ", knockback_direction_difference, " | Dir ", dir, " | Vel.X ", velocity.x)
	
	 # ---- KNOCKDOWN ----
	if final_damage >= 25:
		is_knocked_down = true
		knockdown_time = knockdown_duration
		animated_sprite.play("knockdown")
		return
	
	if apply_stun:
		is_stunned = true
		stun_timer = hitstun_duration
		is_attacking = false
		
		if play_getting_hit_animation:
			animated_sprite.play("p2_get_hit")
		
	else:
		# If blocking, return to blocking idle animation
		if is_crouching:
			animated_sprite.play("p2_block_crouch")
		else:
			animated_sprite.play("p2_block_standing")
	
	
	# Death
	if health <= 0:
		animated_sprite.play("p2_defeat")
		emit_signal("character_died", name)
		die()
	


func die():
	print("PLAYER DEAD")

func _on_hurtbox_area_entered(area: Area2D) -> void:
	
	# We can identify hitboxes by checking their collision layer or by adding them to a group.
	if area.is_in_group("hitbox"):
		
		# Ensure the hitbox belongs to an opponent, not self
		if area.owner != self:
			
			# Pass hitbox position for knockback direction
			take_damage(damage_amount, area.global_position) 
	

func reset_stats():
	health = max_health
	is_stunned = false
	is_attacking = false
	is_blocking = false
	velocity = Vector2.ZERO
	health_changed.emit(health, name)
