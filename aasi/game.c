#include <stdlib.h>
#include <time.h>

#include <aasi/game.h>
#include "screen_obj.h"
#include "so_list.h"
#include "alien.h"
#include "block.h"
#include "bomb.h"
#include "hero.h"
#include "ooc.h"

typedef struct _aasi_game_t {
	struct _aasi_display_t *disp;
	aasi_so_list_t aliens;
	aasi_so_list_t bombs;
	aasi_so_list_t blocks;
	aasi_hero_t *hero;
	unsigned long ts_start;
	unsigned long ts_now;

	aasi_ctxcb_t on_alien_hit;
	aasi_ctxcb_t on_block_destroyed;
	aasi_ctxcb_t on_hero_fire;

	aasi_game_random_provider_t random_provider;
} aasi_game_t;

static const unsigned long _aasi_game_max_time = 30*1000UL / GAME_SPEED_FACTOR;

static void _aasi_game_add_aliens(aasi_game_t *this, int num_aliens) {
	for (int i = 0; i < num_aliens; ++i) {
		aasi_so_list_add(&this->aliens, (aasi_screen_obj_t*)aasi_alien_new(this, i));
	}
}

static bool _aasi_game_block_is_fittable(aasi_game_t *this, aasi_block_t *block) {
	AASI_SO_LIST_FOR_EACH(&this->blocks, fixedblock) {
		if (aasi_screen_obj_is_collision((aasi_screen_obj_t*)block, fixedblock)) {
			aasi_block_delete(block);
			return false;
		}
	}
	return true;
}

static aasi_block_t *_aasi_game_fit_new_block(aasi_game_t *this) {
	aasi_block_t *block = NULL;
	do {
		block = aasi_block_new(this);
	} while (!_aasi_game_block_is_fittable(this, block));
	return block;
}

static void _aasi_game_add_blocks(aasi_game_t *this, int num_blocks) {
	for (int i = 0; i < num_blocks; ++i) {
		aasi_block_t *block = _aasi_game_fit_new_block(this);
		aasi_so_list_add(&this->blocks, (aasi_screen_obj_t*)block);
	}
}

static unsigned int _aasi_game_default_random_provider() {
	static bool random_inited = false;
	if (!random_inited) {
		srand(time(NULL));
		random_inited = true;
	}
	return rand();
}

static bool _aasi_game_init(aasi_game_t *this, struct _aasi_display_t *disp, int num_aliens, int num_blocks) {
	this->disp = disp;
	this->ts_start = 0;
	this->ts_now = 0;
	this->random_provider = _aasi_game_default_random_provider;

	aasi_so_list_init(&this->aliens);
	aasi_so_list_init(&this->blocks);
	aasi_so_list_init(&this->bombs);
	aasi_so_list_init(&this->blocks);
	aasi_ctxcb_init(&this->on_alien_hit);
	aasi_ctxcb_init(&this->on_block_destroyed);
	aasi_ctxcb_init(&this->on_hero_fire);

	_aasi_game_add_aliens(this, num_aliens);
	_aasi_game_add_blocks(this, num_blocks);

	this->hero = aasi_hero_new(this);
	if (!this->hero) {
		aasi_so_list_destroy(&this->aliens);
		aasi_so_list_destroy(&this->blocks);
		return false;
	}
	return true;
}

aasi_game_t* aasi_game_new(struct _aasi_display_t *disp, int num_aliens, int num_blocks) {
	return NEW_INIT(aasi_game_t, _aasi_game_init, disp, num_aliens, num_blocks);
}

void aasi_game_delete(aasi_game_t *this) {
	aasi_hero_delete(this->hero);
	aasi_so_list_destroy(&this->aliens);
	aasi_so_list_destroy(&this->bombs);
	aasi_so_list_destroy(&this->blocks);
	free(this);
}

struct _aasi_display_t *_aasi_game_get_display(aasi_game_t *this) {
	return this->disp;
}

unsigned long aasi_game_get_duration_ms(const aasi_game_t *this) {
	return this->ts_now - this->ts_start;
}

aasi_game_winner_t aasi_game_get_winner(const aasi_game_t *this) {
	if (!aasi_hero_is_alive(this->hero)) {
		if (aasi_so_list_empty(&this->aliens)) {
			return AASI_GAME_WINNER_NO_ONE;
		} else {
			return AASI_GAME_WINNER_ALIENS;
		}
	}

	if (aasi_so_list_empty(&this->aliens)) {
		return AASI_GAME_WINNER_HERO;
	}

	if (aasi_game_get_duration_ms(this) >= _aasi_game_max_time) {
		return AASI_GAME_WINNER_TIME;
	}

	return AASI_GAME_WINNER_UNDETERMINED;
}

bool aasi_game_is_running(const aasi_game_t *this) {
	return aasi_game_get_winner(this) == AASI_GAME_WINNER_UNDETERMINED;
}

void _aasi_game_bomb_new(aasi_game_t *this, aasi_screen_obj_t *source, int y_dir) {
	aasi_so_list_add(&this->bombs, (aasi_screen_obj_t*)aasi_bomb_new(source, y_dir));
}

static void _aasi_game_hero_fire(aasi_game_t *this) {
	_aasi_game_bomb_new(this, (aasi_screen_obj_t*)this->hero, -1);
	aasi_ctxcb_call(&this->on_hero_fire);
}

void aasi_game_handle_key(aasi_game_t *this, aasi_button_t key) {
	switch (key) {
		case AASI_GAME_KEY_DIE:   aasi_hero_kill(this->hero);     break;
		case AASI_GAME_KEY_LEFT:  aasi_hero_move(this->hero, -1); break;
		case AASI_GAME_KEY_RIGHT: aasi_hero_move(this->hero, +1); break;
		case AASI_GAME_KEY_FIRE:  _aasi_game_hero_fire(this);     break;
		case AASI_GAME_KEY_NOT_MAPPED: 						      break;
	}
}

aasi_screen_obj_t* _aasi_game_find_hit_obj(aasi_game_t *this, aasi_bomb_t *bomb) {
	const bool bomb_from_alien = aasi_so_list_find(&this->aliens, (aasi_screen_obj_t*)aasi_bomb_get_source(bomb)) >= 0;
	AASI_SO_LIST_FOR_EACH(&this->aliens, alien) {
		if (aasi_screen_obj_is_collision(alien, (aasi_screen_obj_t*)bomb) && !bomb_from_alien) {
			return alien;
		}
	}
	AASI_SO_LIST_FOR_EACH(&this->blocks, block) {
		if (aasi_screen_obj_is_collision(block, (aasi_screen_obj_t*)bomb)) {
			return block;
		}
	}
	if (aasi_screen_obj_is_collision((aasi_screen_obj_t*)this->hero, (aasi_screen_obj_t*)bomb)) {
		return (aasi_screen_obj_t*)this->hero;
	}
	return NULL;
}

static void _aasi_game_aliens_task(aasi_game_t *this) {
	AASI_SO_LIST_FOR_EACH(&this->aliens, alien) {
		aasi_screen_obj_task(alien);
	}
}

static void _aasi_game_blocks_task(aasi_game_t *this) {
	AASI_SO_LIST_FOR_EACH(&this->blocks, block) {
		aasi_screen_obj_task(block);
	}
}

static void _aasi_game_bombs_task(aasi_game_t *this) {
	AASI_SO_LIST_FOR_EACH(&this->bombs, bomb_so) {
		aasi_bomb_t *const bomb = (aasi_bomb_t*)bomb_so;
		aasi_screen_obj_task(bomb_so);
		if (aasi_bomb_is_off_screen(bomb)) {
			aasi_so_list_erase(&this->bombs, bomb_so);
			continue;
		}

		aasi_screen_obj_t *hit_obj = _aasi_game_find_hit_obj(this, bomb);
		if (hit_obj) {
			aasi_so_list_erase(&this->bombs, bomb_so);
			aasi_screen_obj_hit(hit_obj);
		}
	}
}

void aasi_game_task(aasi_game_t *this, unsigned long timestamp_ms) {
	this->ts_now = timestamp_ms;
	aasi_hero_task(this->hero);
	_aasi_game_aliens_task(this);
	_aasi_game_blocks_task(this);
	_aasi_game_bombs_task(this);
}

void _aasi_game_on_alien_killed(aasi_game_t *this, aasi_alien_t *alien) {
	aasi_so_list_erase(&this->aliens, (aasi_screen_obj_t*)alien);
	aasi_ctxcb_call(&this->on_alien_hit);
}

void _aasi_game_on_block_destroyed(aasi_game_t *this, aasi_block_t *block) {
	aasi_so_list_erase(&this->blocks, (aasi_screen_obj_t*)block);
	aasi_ctxcb_call(&this->on_block_destroyed);
}

void aasi_game_on_alien_hit(aasi_game_t *this, aasi_ctxcb_cb_t cb, void *priv) {
	aasi_ctxcb_set(&this->on_alien_hit, cb, priv);
}

void aasi_game_on_block_destroyed(aasi_game_t *this, aasi_ctxcb_cb_t cb, void *priv) {
	aasi_ctxcb_set(&this->on_block_destroyed, cb, priv);
}

void aasi_game_on_hero_fire(aasi_game_t *this, aasi_ctxcb_cb_t cb, void *priv) {
	aasi_ctxcb_set(&this->on_hero_fire, cb, priv);
}

void aasi_game_set_random_provider(aasi_game_t *game, aasi_game_random_provider_t random_provider) {
	if (random_provider) {
		game->random_provider = random_provider;
	}
}

unsigned int _aasi_game_rand(const aasi_game_t *game) {
	return game->random_provider();
}