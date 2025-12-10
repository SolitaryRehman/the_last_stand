extends Control

@onready var character_select: Control = $"."

#@onready var status_label = $Panel/CenterBox/MainVBox/StatusLabel
@onready var status_label: Label = $Panel/CenterBox/MainVbox/StatusLabel
@onready var character_grid: GridContainer = $Panel/CenterBox/MainVbox/CharacterGrid

@onready var multiplayer_option = $MultiplayerOption
@onready var startbutton = $Panel/CenterBox/MainVbox/BottomButtons/Startbutton
@onready var backbutton: Button = $Panel/CenterBox/MainVbox/BottomButtons/Backbutton

var selecting_player := 1

func _ready():
	startbutton.disabled = true
	status_label.text = "Player 1: Choose Your Fighter"

	#backbutton.pressed.connect(_on_back_pressed)
	startbutton.pressed.connect(_on_start_pressed)

	for btn in character_grid.get_children():
		if btn is TextureButton:
			btn.pressed.connect(_on_character_pressed.bind(btn))

#func _on_back_pressed():
	#get_tree().change_scene_to_file("res://scenes/main_menu.tscn")

func _on_character_pressed(btn):
	var char_path = btn.get_meta("character_path")

	if selecting_player == 1:
		GameManager.p1_character_path = char_path
		status_label.text = "Player 1 locked! Player 2 choose now."
		selecting_player = 2
	elif selecting_player == 2:
		GameManager.p2_character_path = char_path
		status_label.text = "Both players ready!"
		startbutton.disabled = false
		selecting_player = 0

func _on_start_pressed():
	get_tree().change_scene_to_file("res://scenes/game.tscn")


func _on_backbutton_pressed() -> void:
	queue_free()
	get_parent().multiplayer_option.visible = true 
	#get_tree().change_scene_to_file("res://main_menu.tscn")
	
@onready var mapselectscene = preload("res://scenes/map_select.tscn")

func _on_choose_map_pressed() -> void:
	character_select.visible = false
	var mapselectinstance = mapselectscene.instantiate()
	get_tree().current_scene.add_child(mapselectinstance)  # add to root of MainMenu
