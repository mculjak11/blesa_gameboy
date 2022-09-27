#ifndef _AASI_BOMB_H_
#define _AASI_BOMB_H_

#include <stdbool.h>

struct _aasi_bomb_t;
struct _aasi_screen_obj_t;
typedef struct _aasi_bomb_t aasi_bomb_t;

aasi_bomb_t* aasi_bomb_new(const struct _aasi_screen_obj_t *source, int y_dir);
const struct _aasi_screen_obj_t* aasi_bomb_get_source(const aasi_bomb_t *this);
bool aasi_bomb_is_off_screen(const aasi_bomb_t *this);

#endif
