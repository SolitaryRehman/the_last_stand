#include "HealthSystem.h"

using namespace godot;

HealthSystem::HealthSystem() {}
HealthSystem::~HealthSystem() {}

void HealthSystem::_register_methods() {
	// Methods
	register_method("set_health", &HealthSystem::set_health);
	register_method("get_health", &HealthSystem::get_health);
	register_method("set_max_health", &HealthSystem::set_max_health);
	register_method("get_max_health", &HealthSystem::get_max_health);
	register_method("set_immortality", &HealthSystem::set_immortality);
	register_method("get_immortality", &HealthSystem::get_immortality);
	register_method("set_temporary_immortality", &HealthSystem::set_temporary_immortality);

	// Signals
	register_signal<HealthSystem>("max_health_changed", "diff", GODOT_VARIANT_TYPE_INT);
	register_signal<HealthSystem>("health_changed", "diff", GODOT_VARIANT_TYPE_INT);
	register_signal<HealthSystem>("health_depleted");
}

void HealthSystem::_init() {
	_max_health = 3;
	_health = _max_health;
	_immortality = false;
	immortality_timer = nullptr;
}

// ============================================================
// GETTERS / SETTERS
// ============================================================
int HealthSystem::get_max_health() const {
	return _max_health;
}

void HealthSystem::set_max_health(int value) {
	int clamped_value = (value <= 0) ? 1 : value;

	if (clamped_value != _max_health) {
		int difference = clamped_value - _max_health;
		_max_health = clamped_value;
		emit_signal("max_health_changed", difference);
	}

	if (_health > _max_health) {
		_health = _max_health;
	}
}

int HealthSystem::get_health() const {
	return _health;
}

void HealthSystem::set_health(int value) {
	if (_immortality && value < _health) return;

	int clamped_value = CLAMP(value, 0, _max_health);
	if (clamped_value != _health) {
		int difference = clamped_value - _health;
		_health = clamped_value;
		emit_signal("health_changed", difference);

		if (_health == 0) {
			emit_signal("health_depleted");
		}
	}
}

bool HealthSystem::get_immortality() const {
	return _immortality;
}

void HealthSystem::set_immortality(bool value) {
	_immortality = value;
}

// ============================================================
// TEMPORARY IMMORTALITY
// ============================================================
void HealthSystem::set_temporary_immortality(float time) {
	if (!immortality_timer) {
		immortality_timer = Timer::_new();
		immortality_timer->set_one_shot(true);
		add_child(immortality_timer);
	}

	if (immortality_timer->is_connected("timeout", this, "set_immortality")) {
		immortality_timer->disconnect("timeout", this, "set_immortality");
	}

	immortality_timer->set_wait_time(time);
	immortality_timer->connect("timeout", this, "set_immortality", Array::make(false));

	_immortality = true;
	immortality_timer->start();
}
