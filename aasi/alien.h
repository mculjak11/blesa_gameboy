#ifndef _AASI_ALIEN_H_
#define _AASI_ALIEN_H_

struct _aasi_game_t;
struct _aasi_alien_t;
typedef struct _aasi_alien_t aasi_alien_t;

aasi_alien_t* aasi_alien_new(struct _aasi_game_t *game, int height);

#endif
