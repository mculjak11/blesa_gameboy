#include <stdlib.h>
#include <string.h>

#include <aasi/display.h>
#include <aasi/game.h>
#include "screen_obj.h"


static size_t _aasi_screen_obj_get_shape_width(const aasi_screen_obj_t *this) {
	return strlen(this->_shape);
}

bool _aasi_screen_obj_init(aasi_screen_obj_t *this, const aasi_screen_obj_ops_t *ops, aasi_game_t *game, const char *shape, int y, int x) {
	this->priv = NULL;
	this->_ops = ops;
	this->_game = game;
	this->_disp = _aasi_game_get_display(game);
	this->_shape = shape;
	this->_init_draw = true;

	if (y < 0) {
		y += aasi_display_height(this->_disp);
	}
	this->_y = y;

	if (x < 0) {
		x += aasi_display_width(this->_disp);
	}
	this->_x = x;

	size_t shape_width = _aasi_screen_obj_get_shape_width(this);
	char *spaces = (char*)malloc(shape_width + 1);
	if (!spaces) {
		return false;
	}
	memset(spaces, ' ', shape_width);
	spaces[shape_width] = 0;
	this->_spaces = spaces;
	return true;
}


void aasi_screen_obj_delete(aasi_screen_obj_t *this) {
	if (this->_ops && this->_ops->destroy) {
		this->_ops->destroy(this);
	}
	aasi_display_objdel(this->_disp, &this->priv);
	free((char*)this->_spaces);
	free(this);
}

void aasi_screen_obj_hit(aasi_screen_obj_t *this) {
	if (this->_ops && this->_ops->hit) {
		this->_ops->hit(this);
	}
}

void _aasi_screen_obj_draw(aasi_screen_obj_t *this) {
	aasi_display_mvputs(this->_disp, &this->priv, this->_y, this->_x, this->_shape);
}

void aasi_screen_obj_task(aasi_screen_obj_t *this) {
	if (this->_ops && this->_ops->task) {
		this->_ops->task(this);
	}

	if (this->_init_draw) {
		this->_init_draw = false;
		_aasi_screen_obj_draw(this);
	}
}

void _aasi_screen_obj_clear(aasi_screen_obj_t *this) {
	aasi_display_mvclr(this->_disp, &this->priv, this->_y, this->_x, this->_spaces);
}

void _aasi_screen_obj_move_absolute(aasi_screen_obj_t *this, int abs_y, int abs_x) {
	if (!this->_init_draw) {
		_aasi_screen_obj_clear(this);
	}

	this->_x = abs_x;
	this->_y = abs_y;

	const int disp_width = aasi_display_width(this->_disp);
	if (this->_x < 0) {
		this->_x = 0;
	} else if (this->_x >= disp_width) {
		this->_x = disp_width-1;
	}

	const int disp_height = aasi_display_height(this->_disp);
	if (this->_y < 0) {
		this->_y = 0;
	} else if (this->_y >= disp_height) {
		this->_y = disp_height-1;
	}

	if (!this->_init_draw) {
		_aasi_screen_obj_draw(this);
	}
}

void _aasi_screen_obj_move_relative(aasi_screen_obj_t *this, int rel_y, int rel_x) {
	_aasi_screen_obj_move_absolute(this, this->_y + rel_y, this->_x + rel_x);
}

int aasi_screen_obj_get_y(const aasi_screen_obj_t *this) {
	return this->_y;
}

int aasi_screen_obj_get_x(const aasi_screen_obj_t *this) {
	return this->_x;
}

int aasi_screen_obj_max_y(const aasi_screen_obj_t *this) {
	return aasi_display_height(this->_disp);
}

int aasi_screen_obj_max_x(const aasi_screen_obj_t *this) {
	return aasi_display_width(this->_disp);
}

int aasi_screen_obj_get_center(const aasi_screen_obj_t *this) {
	return this->_x + _aasi_screen_obj_get_shape_width(this) / 2;
}

bool aasi_screen_obj_is_collision(const aasi_screen_obj_t *this, const aasi_screen_obj_t *other) {
	return
		this->_y == other->_y &&
		this->_x <= (other->_x + _aasi_screen_obj_get_shape_width(other) - 1) &&
		(this->_x + _aasi_screen_obj_get_shape_width(this) - 1) >= other->_x;
}

unsigned long _aasi_screen_obj_millis(const aasi_screen_obj_t *this) {
	return aasi_game_get_duration_ms(this->_game);
}

bool _aasi_screen_obj_is_timeout(const aasi_screen_obj_t *this, unsigned long ts_start, unsigned long interval) {
	return _aasi_screen_obj_millis(this) - ts_start >= interval;
}

unsigned int _aasi_screen_obj_rand(const aasi_screen_obj_t *this) {
	return _aasi_game_rand(this->_game);
}
