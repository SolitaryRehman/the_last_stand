extends Node2D

@onready var player_1: CharacterBody2D = $Player1
@onready var player_2: CharacterBody2D = $player2

@onready var health_bar_player_1: ProgressBar = $UI/health_bar_player_1
@onready var health_bar_player_2: ProgressBar = $UI/health_bar_player_2

@onready var round_timer_label: Label = $UI/round_timer
@onready var round_counter_label: Label = $UI/round_counter
@onready var match_state_msgs: Label = $UI/match_state_msgs
@onready var round_timer: Timer = $round_timer

@onready var victory_screen :Panel = $UI/victory_screen
@onready var victory_label :Label = $UI/victory_screen/victory_label
@onready var restart_button :Button = $UI/victory_screen/restart_button


enum State { IDLE, INTRO, FIGHT, ROUND_END, MATCH_END }
var current_state = State.IDLE

var rounds_won_p1: int = 0
var rounds_won_p2: int = 0
const MAX_ROUNDS = 3


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	
	# Connect health_changed signals from both players
	player_1.health_changed.connect(on_player_health_changed)
	player_2.health_changed.connect(on_player_health_changed)

			# Initialize health bars
	health_bar_player_1.value = player_1.health
	health_bar_player_2.value = player_2.health
	
	player_1.character_died.connect(on_character_died)
	player_2.character_died.connect(on_character_died)
	
	start_match()
	
	restart_button.pressed.connect(on_restart_button_pressed)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if current_state == State.FIGHT:
		round_timer_label.text = str(int(round_timer.time_left))


func on_player_health_changed(new_health: int, character_id: String):
	
	if character_id == player_1.name:
		health_bar_player_1.value = new_health
	elif character_id == player_2.name:
		health_bar_player_2.value = new_health
	

func start_match():
	
	current_state = State.INTRO
	match_state_msgs.text = "Round 1"
	match_state_msgs.visible = true

	# Use a Tween to animate players into position
	var tween = create_tween()
	tween.tween_property(player_1, "position:x", 200, 1.0).from(-200) # Animate from off-screen
	tween.tween_property(player_2, "position:x", 1080, 1.0).from(1480)

		# After the intro animation, start the fight
	tween.tween_callback(start_fight)


func _on_round_timer_timeout() -> void:
	if current_state == State.FIGHT:
		end_round()


func start_fight():
	current_state = State.FIGHT
	match_state_msgs.text = "Fight!"
	# Use a timer to hide the "Fight!" message after a second
	get_tree().create_timer(1.0).timeout.connect(func(): match_state_msgs.visible = false)
	round_timer.start()
	

func on_character_died(character_id: String):
	if current_state == State.FIGHT:
		end_round()

func end_round():
	current_state = State.ROUND_END
	round_timer.stop()

	if player_1.health > player_2.health:
		rounds_won_p1 += 1
		match_state_msgs.text = "Player 1 Wins Round!"
	elif player_2.health > player_1.health:
		rounds_won_p2 += 1
		match_state_msgs.text = "Player 2 Wins Round!"
	else:
		match_state_msgs.text = "Draw!"

	match_state_msgs.visible = true
	round_counter_label.text = str(rounds_won_p1) + " - " + str(rounds_won_p2)
	
	if rounds_won_p1 >= 2 or rounds_won_p2 >= 2:
		end_match()
	else:
		# Start next round after a delay
		get_tree().create_timer(3.0).timeout.connect(reset_round)


func reset_round():
		# Reset player health, energy, position
	player_1.reset_stats()
	player_2.reset_stats()
	player_1.position = Vector2(200, 500)
	player_2.position = Vector2(1080, 500)
	player_1.show()
	player_2.show()

		# Start next round
	start_fight()


func end_match():
	current_state = State.MATCH_END
	victory_screen.visible = true

	if rounds_won_p1 > rounds_won_p2:
		victory_label.text = "Player 1 Wins!"
	else:
		victory_label.text = "Player 2 Wins!"


func on_restart_button_pressed():
	get_tree().reload_current_scene()
