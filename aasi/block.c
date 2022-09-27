#include <aasi/game.h>
#include "screen_obj.h"
#include "block.h"
#include "ooc.h"

struct _aasi_block_t {
	aasi_screen_obj_t so;
	int hp;
};

static void _aasi_block_hit(aasi_screen_obj_t *base);

static const int aasi_block_init_hit_points = 4;
static const char _aasi_block_shape[] = "[XX]";
static const aasi_screen_obj_ops_t _aasi_block_ops = {
	.hit = _aasi_block_hit,
};

static bool _aasi_block_init(aasi_block_t *this, struct _aasi_game_t *game) {
	if (!_aasi_screen_obj_init(&this->so, &_aasi_block_ops, game, _aasi_block_shape, 0, 0)) {
		return false;
	}
	this->hp = aasi_block_init_hit_points;
	const int half_y = aasi_screen_obj_max_y(&this->so) / 2;
	const int rnd_x = _aasi_screen_obj_rand(&this->so) 
					% (aasi_screen_obj_max_x(&this->so) - sizeof(_aasi_block_shape));
	_aasi_screen_obj_move_absolute(&this->so, half_y, rnd_x);
	return true;
}

aasi_block_t *aasi_block_new(struct _aasi_game_t *game) {
	return NEW_INIT(aasi_block_t, _aasi_block_init, game);
}

void _aasi_block_hit(aasi_screen_obj_t *base) {
	aasi_block_t *this = (aasi_block_t*)base;
	this->hp--;
	if (this->hp <= 0) {
		_aasi_screen_obj_clear(base);
		_aasi_game_on_block_destroyed(base->_game, this);
	} else {
		_aasi_screen_obj_draw(base);
	}
}

void aasi_block_delete(aasi_block_t *this) {
	aasi_screen_obj_delete((aasi_screen_obj_t*)this);
}
