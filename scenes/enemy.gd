# The moving Character enemy
extends Node2D

const SPEED = 45

var health
var direction = 1

@onready var ray_cast_right: RayCast2D = $RayCastRight
@onready var ray_cast_left: RayCast2D = $RayCastLeft
@onready var animated_sprite: AnimatedSprite2D = $AnimatedSprite2D
@onready var killzone: Area2D = $killzone
@onready var health_bar: ProgressBar = $"health bar"
@onready var attack_hitbox: Area2D = $"attack hitbox"


func _ready() -> void:
	health_bar.init_health(health)

	killzone.area_entered.connect(_on_killzone_area_entered)
	attack_hitbox.area_shape_entered.connect(_on_attack_hitbox_area_shape_entered)



# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	if ray_cast_right.is_colliding():
		direction = -1
		animated_sprite.flip_h = true
	if ray_cast_left.is_colliding():
		direction = 1
		animated_sprite.flip_h = false
	position.x += direction * SPEED * delta 
	
	if direction != 0:
		animated_sprite.play("walk")
	




func _on_attack_hitbox_area_shape_entered(area_rid: RID, area: Area2D, area_shape_index: int, local_shape_index: int) -> void:
	health -= 1
	health = max(health, 0)
	health_bar.value = health
	print ("Enemy took damage")  # update bar

	if health <= 0:
		queue_free()


func _on_killzone_area_entered(area):
	if area.has_method("take_damage"):
		area.take_damage(1)  # kill enemy
