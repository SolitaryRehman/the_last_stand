extends Node2D


@onready var player_1: CharacterBody2D = $Player1
@onready var player_2: CharacterBody2D = $player2

@onready var camera_2d: Camera2D = $Camera2D


#CAMERA LIMITS SO THAT THE CAMERA DOESN'T GO OUT OF RANGE :---

const MIN_ZOOM = 1.0          # Closest zoom in
const MAX_ZOOM = 2.5          # Max zoom out
const MAX_DISTANCE = 600.0    # When players are far, zoom out
const MIN_DISTANCE = 150.0    # When close, zoom in

# How fast camera moves/zooms :---
const SMOOTH_SPEED = 5.0


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if not player_1 or player_2:
		return
	
	# =============================
	# 1. FIND MIDPOINT BETWEEN PLAYERS
	# =============================
	var midpoint = (player_1.global_position + player_2.global_position) / 2.0
	
	# Smoothly follow midpoint
	global_position = global_position.lerp(midpoint, delta * SMOOTH_SPEED)
	# lerp function for smooth transition
	# =============================
	# 2. DISTANCE BETWEEN PLAYERS
	# =============================
	var distance = player_1.global_position.distance_to(player_2.global_position)
	
	# Convert distance to zoom
	var target_zoom = clamp(distance / MAX_DISTANCE, MIN_ZOOM, MAX_ZOOM)
	# clamp function for restriction b/w two points
	
	# Smooth zoom
	camera_2d.zoom = camera_2d.zoom.lerp(Vector2(target_zoom, target_zoom), delta * SMOOTH_SPEED)
	
	# =============================
	# 3. BLOCK PLAYERS FROM LEAVING THE SCREEN
	# =============================
	_enforce_bounds()


func _enforce_bounds():
	# Get screen size
	var viewport := get_viewport_rect().size

	# Camera world size after zoom
	var w = viewport.x * camera_2d.zoom.x / 2
	var h = viewport.y * camera_2d.zoom.y / 2

	# Define camera boundaries
	var left_bound = global_position.x - w
	var right_bound = global_position.x + w
	var top_bound = global_position.y - h
	var bottom_bound = global_position.y + h

	# Clamp Player1
	player_1.global_position.x = clamp(player_1.global_position.x, left_bound, right_bound)
	player_1.global_position.y = clamp(player_1.global_position.y, top_bound, bottom_bound)

	# Clamp Player2
	player_2.global_position.x = clamp(player_2.global_position.x, left_bound, right_bound)
	player_2.global_position.y = clamp(player_2.global_position.y, top_bound, bottom_bound)
