extends Control

@onready var character_select: Control = $"."   # root
@onready var status_label: Label = $Panel/CenterBox/MainVbox/StatusLabel
@onready var character_grid: GridContainer = $Panel/CenterBox/MainVbox/CharacterGrid

@onready var multiplayer_option = $MultiplayerOption
@onready var startbutton = $Panel/CenterBox/MainVbox/BottomButtons/Startbutton
@onready var backbutton: Button = $Panel/CenterBox/MainVbox/BottomButtons/Backbutton

var selecting_player := 1  # 1 -> p1, 2 -> p2, 0 -> done
# Preload the MapSelect scene
# Preload your Map scene
#@onready var MapScene = preload("res://Project_B_Content/scenes/game_1.tscn")  # change path to your actual Map scene



func _ready():
	startbutton.disabled = true
	status_label.text = "Player 1: Choose Your Fighter"

	# Connect buttons
	startbutton.pressed.connect(_on_start_pressed)
	backbutton.pressed.connect(_on_backbutton_pressed)

	for btn in character_grid.get_children():
		if btn is TextureButton:
			# bind the button instance so we know which button was pressed
			btn.pressed.connect(_on_character_pressed.bind(btn))

func _on_character_pressed(btn):
	var char_path = btn.get_meta("character_path")
	if not char_path:
		push_error("TextureButton missing 'character_path' metadata!")
		return

	if selecting_player == 1:
		GameManager.p1_character_path = char_path
		status_label.text = "Player 1 locked! Player 2 choose now."
		selecting_player = 2
		# optional: visual feedback
		btn.modulate = Color(0.8,0.8,1)  # slight tint to indicate locked
	elif selecting_player == 2:
		# optionally prevent choosing same char twice
		if char_path == GameManager.p1_character_path:
			status_label.text = "Character already chosen by Player 1!"
			return
		GameManager.p2_character_path = char_path
		status_label.text = "Both players ready!"
		startbutton.disabled = false
		selecting_player = 0
		btn.modulate = Color(0.8,0.8,1)

func _on_start_pressed():
	# ensure both are set
	#if not GameManager.p1_character_path or not GameManager.p2_character_path:
		#status_label.text = "Select both characters!"
		#return
	## change to the game/map scene (your map scene path)
	#get_tree().change_scene_to_file("res://scenes/map1.tscn")
	## Create an instance of the Map scene
	#var map_instance = MapScene.instantiate()
	get_tree().change_scene_to_file("res://Project_B_Content/scenes/game_1.tscn")
	# Set it as the current scene
	#get_tree().current_scene = map_instance


func _on_backbutton_pressed() -> void:
	# show multiplayer options in parent, then free this select screen
	var parent = get_parent()
	if parent:
		var mo = parent.get_node_or_null("MultiplayerOption")
		if mo:
			mo.visible = true
	queue_free()
