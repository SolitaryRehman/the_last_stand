extends Control
@onready var main_buttons: VBoxContainer = $MainButtons
@onready var options: Panel = $Options
@onready var play: Panel = $play
@onready var multiplayer_option: Panel = $MultiplayerOption

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	main_buttons.visible = true
	options.visible = false
	play.visible = false
	#multiplayer_option.visibile = false


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass


func _on_start_pressed() -> void:
	main_buttons.visible = false
	play.visible = true


func _on_settings_pressed() -> void:
	print("Settings Pressed")
	main_buttons.visible = false
	options.visible = true


func _on_exit_pressed() -> void:
	get_tree().quit()

func _on_back_options_pressed() -> void:
	_ready()


func _on_back_start_pressed() -> void:
	_ready()


func _on_multiplayermode_pressed() -> void:
	play.visible = false
	multiplayer_option.visible = true
	


func _on_back_pressed() -> void:
	multiplayer_option.visible = false
	play.visible = true



# Preload the CharacterSelect scene
@onready var character_select_scene = preload("res://scenes/character_select.tscn")

func _on_offline_pvp_pressed() -> void:
	multiplayer_option.visible = false
	var character_select_instance = character_select_scene.instantiate()
	add_child(character_select_instance)
