extends Node

# Player character paths
var p1_character_path : String = "res://scenes/player1.tscn"
var p2_character_path : String = "res://scenes/player1.tscn"

# Optional: store scores or other global info
var p1_score : int = 0
var p2_score : int = 0

# Optional: store game settings
var game_mode : String = "PvP"  # could be "PvE" later
var current_level : int = 1
