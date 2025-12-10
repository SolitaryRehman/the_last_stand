# res://scenes/map_select.gd
extends Control
@onready var map_grid: GridContainer = $Panel/CenterBox/MainVBox/HBoxContainer/MapGrid
@onready var map_preview: TextureRect = $Panel/CenterBox/MainVBox/HBoxContainer/MapPreview

#@onready var grid = $Panel/CenterBox/MainVBox/HBoxTop/MapGrid
@onready var preview = $Panel/CenterBox/MainVBox/HBoxTop/MapPreview
@onready var status_label = $Panel/CenterBox/MainVBox/StatusLabel
@onready var start_button = $Panel/CenterBox/MainVBox/BottomButtons/StartButton
@onready var back_button = $Panel/CenterBox/MainVBox/BottomButtons/BackButton


var selected_map_button: TextureButton = null

func _ready():
	start_button.disabled = true
	status_label.text = "Choose a map"

	for btn in map_grid.get_children():
		if btn is TextureButton:
			btn.pressed.connect(_on_map_pressed.bind(btn))
			btn.mouse_entered.connect(_on_map_hover.bind(btn))
			btn.mouse_exited.connect(_on_map_unhover.bind(btn))

#func _on_back_pressed():
	## return to character select to change picks
	#get_tree().change_scene_to_file("res://scenes/character_select.tscn")

func _on_map_pressed(btn):
	var mp := ""
	if btn.has_meta("map_path"):
		mp = btn.get_meta("map_path")
	if mp == "":
		push_warning("map_path not set for %s" % btn.name)
		return

	GameManager.selected_map_path = mp
	selected_map_button = btn
	_highlight_button(btn)
	start_button.disabled = false
	status_label.text = "Map selected: %s" % btn.name

	# set preview if meta provided
	if btn.has_meta("map_preview"):
		var tex = load(btn.get_meta("map_preview"))
		if tex:
			map_preview.texture = tex

func _on_map_hover(btn):
	if btn.has_meta("map_preview"):
		var tex = load(btn.get_meta("map_preview"))
		if tex:
			map_preview.texture = tex

func _on_map_unhover(btn):
	# show either selected preview or clear
	if selected_map_button and selected_map_button.has_meta("map_preview"):
		map_preview.texture = load(selected_map_button.get_meta("map_preview"))
	else:
		map_preview.texture = null


func _highlight_button(btn):
	for b in map_grid.get_children():
		if b is TextureButton:
			b.modulate = Color(1,1,1,1)
	btn.modulate = Color(1,0.9,0.6,1)
	
	
@onready var character_select_scene = preload("res://scenes/character_select.gd")



func _on_back_button_pressed() -> void:
	get_parent().character_select.visible = true
	queue_free()
