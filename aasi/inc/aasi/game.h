#ifndef _AASI_GAME_H_
#define _AASI_GAME_H_

#include <stdbool.h>
#include <aasi/ctxcb.h>

#define GAME_SPEED_FACTOR 4
typedef enum
{
	AASI_GAME_KEY_NOT_MAPPED = -1,
    AASI_GAME_KEY_LEFT,
	AASI_GAME_KEY_RIGHT,
	AASI_GAME_KEY_FIRE,
	AASI_GAME_KEY_DIE,
} aasi_button_t;



struct _aasi_game_t;
typedef struct _aasi_game_t aasi_game_t;
struct _aasi_display_t;

typedef enum _aasi_game_winner_t {
	AASI_GAME_WINNER_UNDETERMINED = 0,	// game is still running
	AASI_GAME_WINNER_HERO,				// hero wins
	AASI_GAME_WINNER_ALIENS,			// hero looses
	AASI_GAME_WINNER_TIME,				// timeout
	AASI_GAME_WINNER_NO_ONE,			// quit
} aasi_game_winner_t;

typedef unsigned int (*aasi_game_random_provider_t)();

aasi_game_t* aasi_game_new(struct _aasi_display_t *disp, int num_aliens, int num_blocks);
bool aasi_game_is_running(const aasi_game_t *this);
void aasi_game_task(aasi_game_t *this, unsigned long timestamp_ms);
void aasi_game_delete(aasi_game_t *this);
void aasi_game_handle_key(aasi_game_t *this, aasi_button_t key);
aasi_game_winner_t aasi_game_get_winner(const aasi_game_t *this);
unsigned long aasi_game_get_duration_ms(const aasi_game_t *this);
void aasi_game_on_alien_hit(aasi_game_t *this, aasi_ctxcb_cb_t cb, void *priv);
void aasi_game_on_block_destroyed(aasi_game_t *this, aasi_ctxcb_cb_t cb, void *priv);
void aasi_game_on_hero_fire(aasi_game_t *this, aasi_ctxcb_cb_t cb, void *priv);
void aasi_game_set_random_provider(aasi_game_t *this, aasi_game_random_provider_t rnd);

// protected, for screen_obj_t based objects only
struct _aasi_alien_t;
struct _aasi_block_t;
struct _aasi_screen_obj_t;
void _aasi_game_on_alien_killed(aasi_game_t *this, struct _aasi_alien_t *alien);
void _aasi_game_on_block_destroyed(aasi_game_t *this, struct _aasi_block_t *alien);
void _aasi_game_bomb_new(aasi_game_t *this, struct _aasi_screen_obj_t *source, int y_dir);
struct _aasi_display_t *_aasi_game_get_display(aasi_game_t *this);
unsigned int _aasi_game_rand(const aasi_game_t *game);

#endif
