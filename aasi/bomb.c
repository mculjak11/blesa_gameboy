#include <aasi/display.h>
#include "screen_obj.h"
#include "bomb.h"
#include "ooc.h"

static const unsigned long aasi_bomb_interval = 40;

struct _aasi_bomb_t {
	aasi_screen_obj_t so;
	int y_dir;
	const aasi_screen_obj_t *src;
	unsigned long ts;
	bool offscreen;
};

static void _aasi_bomb_task(aasi_screen_obj_t *this);

static const char _aasi_bomb_shape[] = "o";
static const aasi_screen_obj_ops_t _aasi_bomb_ops = {
	.task = _aasi_bomb_task,
};

bool _aasi_bomb_init(aasi_bomb_t *this, const aasi_screen_obj_t *source, int y_dir) {
	if (y_dir < 0) {
		this->y_dir = -1;
	} else if (y_dir > 0) {
		this->y_dir = 1;
	} else {
		return false;
	}
	this->src = source;
	this->ts = _aasi_screen_obj_millis(source);
	this->offscreen = false;

	const int y = aasi_screen_obj_get_y(source) + this->y_dir;
	const int x = aasi_screen_obj_get_center(source);
	return _aasi_screen_obj_init(&this->so, &_aasi_bomb_ops, source->_game, _aasi_bomb_shape, y, x);
}

aasi_bomb_t* aasi_bomb_new(const aasi_screen_obj_t *source, int y_dir) {
	return NEW_INIT(aasi_bomb_t, _aasi_bomb_init, source, y_dir);
}

const aasi_screen_obj_t* aasi_bomb_get_source(const aasi_bomb_t *this) {
	return this->src;
}

void _aasi_bomb_task(aasi_screen_obj_t *base) {
	aasi_bomb_t *const this = (aasi_bomb_t*)base;
	if (aasi_bomb_is_off_screen(this) ||
	    !_aasi_screen_obj_is_timeout(base, this->ts, aasi_bomb_interval))
	{
		return;
	}
	this->ts = _aasi_screen_obj_millis(base);

	const int new_y = aasi_screen_obj_get_y(base) + this->y_dir;
	if (new_y < 0 || new_y >= aasi_screen_obj_max_y(base)) {
		_aasi_screen_obj_clear(base);
		this->offscreen = true;
	} else {
		_aasi_screen_obj_move_relative(base, this->y_dir, 0);
	}
}

bool aasi_bomb_is_off_screen(const aasi_bomb_t *this) {
	return this->offscreen;
}
