extends Node2D

@onready var player_1: CharacterBody2D = $"../Player1"
@onready var player_2: CharacterBody2D = $"../player2"

@onready var camera_2d: Camera2D = $Camera2D


@export var zoom_in := 1.25    # when players are close
@export var zoom_out := 0.7   # when players are far



func _process(delta):
	if not player_1 or not player_2:
		return
	

	var p1 = player_1.global_position
	var p2 = player_2.global_position

	var midpoint = (p1 + p2) * 0.5
	camera_2d.global_position = midpoint

	var dist = p1.distance_to(p2)

	# NOTE: remap CLOSE -> zoom_in, FAR -> zoom_out
	var target_zoom = clamp(
		remap(dist, 800, 150, zoom_out, zoom_in), zoom_out, zoom_in)

	camera_2d.zoom = camera_2d.zoom.lerp(Vector2(target_zoom, target_zoom), delta * 5)
