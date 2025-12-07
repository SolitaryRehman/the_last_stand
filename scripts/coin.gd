# the pickup coin script
extends Area2D

@onready var game_manager: Node = %"game manager"
@onready var animation_player: AnimationPlayer = $AnimationPlayer


func _on_body_entered(body):
	game_manager.add_point()
	animation_player.play("coin_pickup")
