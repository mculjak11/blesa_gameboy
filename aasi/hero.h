#ifndef _AASI_HERO_H_
#define _AASI_HERO_H_

#include <stdbool.h>

struct _aasi_hero_t;
typedef struct _aasi_hero_t aasi_hero_t;
struct _aasi_game_t;

aasi_hero_t *aasi_hero_new(struct _aasi_game_t *game);
bool aasi_hero_is_alive(const aasi_hero_t *this);
void aasi_hero_delete(aasi_hero_t *this);
void aasi_hero_kill(aasi_hero_t *this);
void aasi_hero_task(aasi_hero_t *this);
void aasi_hero_move(aasi_hero_t *this, int dir);

#endif
