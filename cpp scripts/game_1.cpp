#include "MatchManager.h"

using namespace godot;

MatchManager::MatchManager() {}
MatchManager::~MatchManager() {}

void MatchManager::_register_methods() {
	register_method("_ready", &MatchManager::_ready);
	register_method("_process", &MatchManager::_process);
	register_method("on_player_health_changed", &MatchManager::on_player_health_changed);
	register_method("on_character_died", &MatchManager::on_character_died);
	register_method("on_restart_button_pressed", &MatchManager::on_restart_button_pressed);
	register_method("_on_round_timer_timeout", &MatchManager::_on_round_timer_timeout);

	register_method("start_match", &MatchManager::start_match);
	register_method("start_fight", &MatchManager::start_fight);
	register_method("end_round", &MatchManager::end_round);
	register_method("reset_round", &MatchManager::reset_round);
	register_method("end_match", &MatchManager::end_match);

	register_property<MatchManager, float>("speed", &MatchManager::speed, 0.1);
}

void MatchManager::_init() {
	current_state = IDLE;
	rounds_won_p1 = 0;
	rounds_won_p2 = 0;
	speed = 0.1;
}

void MatchManager::_ready() {
	player_1 = cast_to<CharacterBody2D>(get_node("player_1"));
	player_2 = cast_to<CharacterBody2D>(get_node("player_2"));

	health_bar_player_1 = cast_to<ProgressBar>(get_node("UI/health_bar_player_1"));
	health_bar_player_2 = cast_to<ProgressBar>(get_node("UI/health_bar_player_2"));

	round_timer_label = cast_to<Label>(get_node("UI/round_timer"));
	round_counter_label = cast_to<Label>(get_node("UI/round_counter"));
	match_state_msgs = cast_to<Label>(get_node("UI/match_state_msgs"));

	round_timer = cast_to<Timer>(get_node("round_timer"));

	victory_screen = cast_to<Panel>(get_node("UI/victory_screen"));
	restart_button = cast_to<Button>(get_node("UI/victory_screen/restart_button"));

	// Connect signals
	player_1->get("health_changed").connect("health_changed", Callable(this, "on_player_health_changed"));
	player_2->get("health_changed").connect("health_changed", Callable(this, "on_player_health_changed"));

	player_1->get("character_died").connect("character_died", Callable(this, "on_character_died"));
	player_2->get("character_died").connect("character_died", Callable(this, "on_character_died"));

	restart_button->connect("pressed", Callable(this, "on_restart_button_pressed"));

	health_bar_player_1->set_value(player_1->get("health"));
	health_bar_player_2->set_value(player_2->get("health"));

	victory_screen->set_visible(false);

	start_match();
}

void MatchManager::_process(double delta) {
	if (current_state == FIGHT) {
		round_timer_label->set_text(String::num_int64(int(round_timer->get_time_left())));
	}
}

// ------------------ PLAYER HEALTH --------------------
void MatchManager::on_player_health_changed(int new_health, String character_id) {
	if (character_id == player_1->get("character_name")) {
		health_bar_player_1->set_value(new_health);
	} else if (character_id == player_2->get("character_name")) {
		health_bar_player_2->set_value(new_health);
	}
}

// ------------------ START MATCH --------------------
void MatchManager::start_match() {
	current_state = INTRO;
	match_state_msgs->set_text("Round 1");
	match_state_msgs->set_visible(true);

	Tween* tween = Tween::_new();
	add_child(tween);

	tween->tween_property(player_1, "position:x", 1701.0, 1.0).from(1701.0);
	tween->tween_property(player_2, "position:x", 2127.0, 1.0).from(2127.0);

	tween->tween_callback(Callable(this, "start_fight"));
}

// ------------------ ROUND TIMER --------------------
void MatchManager::_on_round_timer_timeout() {
	if (current_state == FIGHT) {
		end_round();
	}
}

// ------------------ START FIGHT --------------------
void MatchManager::start_fight() {
	current_state = FIGHT;
	match_state_msgs->set_text("Fight!");

	// Hide "Fight!" after 1 second
	get_tree()->create_timer(1.0)->timeout.connect([this](){ match_state_msgs->set_visible(false); });

	round_timer->start();
}

// ------------------ CHARACTER DIED --------------------
void MatchManager::on_character_died(String character_id) {
	if (current_state == FIGHT) {
		end_round();
	}
}

// ------------------ END ROUND --------------------
void MatchManager::end_round() {
	current_state = ROUND_END;
	round_timer->stop();

	if (player_1->get("health") > player_2->get("health")) {
		rounds_won_p1++;
		match_state_msgs->set_text("Geralt Wins Round!");
	} else if (player_2->get("health") > player_1->get("health")) {
		rounds_won_p2++;
		match_state_msgs->set_text("Ciri Wins Round!");
	} else {
		match_state_msgs->set_text("Draw!");
	}

	match_state_msgs->set_visible(true);
	round_counter_label->set_text(String::num_int64(rounds_won_p1) + " - " + String::num_int64(rounds_won_p2));

	if (rounds_won_p1 >= 2 || rounds_won_p2 >= 2) {
		end_match();
	} else {
		get_tree()->create_timer(3.0)->timeout.connect(Callable(this, "reset_round"));
	}
}

// ------------------ RESET ROUND --------------------
void MatchManager::reset_round() {
	player_1->call("reset_stats");
	player_2->call("reset_stats");

	player_1->set_position(Vector2(1701.0, -446.0));
	player_2->set_position(Vector2(2127.0, -430.0));

	player_1->show();
	player_2->show();

	start_fight();
}

// ------------------ END MATCH --------------------
void MatchManager::end_match() {
	current_state = MATCH_END;
	victory_screen->set_visible(true);
	restart_button->set_visible(true);

	Tween* tween = Tween::_new();
	add_child(tween);
	victory_screen->set_modulate(Color(1,1,1,0));
	tween->tween_property(victory_screen, "modulate:a", 1.0, 0.8);
}

// ------------------ RESTART BUTTON --------------------
void MatchManager::on_restart_button_pressed() {
	get_tree()->reload_current_scene();
}
