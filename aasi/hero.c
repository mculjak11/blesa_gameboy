#include <stdlib.h>

#include <aasi/display.h>
#include "screen_obj.h"
#include "hero.h"
#include "ooc.h"

struct _aasi_hero_t {
	aasi_screen_obj_t so;
	bool alive;
};

//static void _aasi_hero_task(aasi_screen_obj_t *base);
static void _aasi_hero_hit(aasi_screen_obj_t *base);

static const char _aasi_hero_shape[] = "A";
static const aasi_screen_obj_ops_t _aasi_hero_ops = {
	.hit = _aasi_hero_hit,
};

static bool _aasi_hero_init(aasi_hero_t *this, struct _aasi_game_t *game) {
	if (!_aasi_screen_obj_init(&this->so, &_aasi_hero_ops, game, _aasi_hero_shape, -1, 0)) {
		return false;
	}
	const int rnd_x = _aasi_screen_obj_rand(&this->so) 
						% aasi_screen_obj_max_x(&this->so);
	_aasi_screen_obj_move_relative(&this->so, 0, rnd_x);
	this->alive = true;
	return true;
}

aasi_hero_t *aasi_hero_new(struct _aasi_game_t *game) {
	return NEW_INIT(aasi_hero_t, _aasi_hero_init, game);
}

bool aasi_hero_is_alive(const aasi_hero_t *this) {
	return this->alive;
}

void aasi_hero_delete(aasi_hero_t *this) {
	aasi_screen_obj_delete(&this->so);
}

void aasi_hero_kill(aasi_hero_t *this) {
	aasi_screen_obj_hit((aasi_screen_obj_t*)this);
}

void _aasi_hero_hit(aasi_screen_obj_t *base) {
	aasi_hero_t *const this = (aasi_hero_t*)base;
	this->alive = false;
}

void aasi_hero_task(aasi_hero_t *this) {
	aasi_screen_obj_task(&this->so);
}

void aasi_hero_move(aasi_hero_t *this, int dir) {
	if (dir < 0) {
		dir = -1;
	} else if (dir > 0) {
		dir = 1;
	}
	_aasi_screen_obj_move_relative(&this->so, 0, dir);
}
