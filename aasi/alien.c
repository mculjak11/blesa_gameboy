#include <stdbool.h>

#include <aasi/display.h>
#include <aasi/game.h>
#include "screen_obj.h"
#include "alien.h"
#include "ooc.h"


static const unsigned long _aasi_alien_interval = 50;

struct _aasi_alien_t {
	aasi_screen_obj_t so;
	int destination;
	unsigned long ts;
};

static void _aasi_alien_task(aasi_screen_obj_t *this);
static void _aasi_alien_hit(aasi_screen_obj_t *this);

static const char _aasi_alien_shape[] = "<____>";
static const aasi_screen_obj_ops_t _aasi_alien_ops = {
	.hit  = _aasi_alien_hit,
	.task = _aasi_alien_task,
};

static void _aasi_alien_pick_destination(aasi_alien_t *this) {
	this->destination = _aasi_screen_obj_rand(&this->so) % (aasi_screen_obj_max_x(&this->so) - sizeof(_aasi_alien_shape));
}

static bool _aasi_alien_init(aasi_alien_t *this, aasi_game_t *game, int height) {
	if (!_aasi_screen_obj_init(&this->so, &_aasi_alien_ops, game, _aasi_alien_shape, 0, 0)) {
		return false;
	}
	_aasi_alien_pick_destination(this);
	this->ts = _aasi_screen_obj_millis(&this->so) + _aasi_alien_interval;
	_aasi_screen_obj_move_absolute(&this->so, height, this->destination);
	return true;
}

aasi_alien_t* aasi_alien_new(aasi_game_t *game, int height) {
	aasi_alien_t *this = NEW(aasi_alien_t);
	if (this) {
		if (!_aasi_alien_init(this, game, height)) {
			free(this);
			this = NULL;
		}
	}
	return this;
}

void _aasi_alien_task(aasi_screen_obj_t *base) {
	aasi_alien_t *const this = (aasi_alien_t*)base;

	if (!_aasi_screen_obj_is_timeout(base, this->ts, _aasi_alien_interval)) {
		return;
	}
	this->ts = _aasi_screen_obj_millis(base);

	const int cur_x = aasi_screen_obj_get_x(base);
	if (cur_x == this->destination) {
		_aasi_alien_pick_destination(this);
	}

	int dir = this->destination - cur_x;
	if (dir < 0) {
		dir = -1;
	} else if (dir > 0) {
		dir = 1;
	}
	_aasi_screen_obj_move_relative(&this->so, 0, dir);

	if (_aasi_screen_obj_rand(&this->so) % 20 == 1) {
		_aasi_game_bomb_new(base->_game, base, 1);
	}
}

void _aasi_alien_hit(aasi_screen_obj_t *base) {
	aasi_alien_t *const this = (aasi_alien_t*)base;
	_aasi_screen_obj_clear(base);
	_aasi_game_on_alien_killed(base->_game, this);
}
