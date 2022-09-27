#ifndef _AASI_SCREEN_OBJ_H_
#define _AASI_SCREEN_OBJ_H_

#include <stdbool.h>

struct _aasi_screen_obj_t;
typedef struct _aasi_screen_obj_t aasi_screen_obj_t;
struct _aasi_display_t;
struct _aasi_game_t;

typedef struct _aasi_screen_obj_ops_t {
	void (*hit)(aasi_screen_obj_t *this);
	void (*task)(aasi_screen_obj_t *this);
	void (*destroy)(aasi_screen_obj_t *this);
} aasi_screen_obj_ops_t;

struct _aasi_screen_obj_t {
	// public:
	void *priv;

	// protected:
	struct _aasi_game_t *_game;

	// private:
	const aasi_screen_obj_ops_t *_ops;
	struct _aasi_display_t *_disp;
	const char *_shape;
	const char *_spaces;
	int _x;
	int _y;
	bool _init_draw;
};

// public:
void aasi_screen_obj_delete(aasi_screen_obj_t *this);
void aasi_screen_obj_task(aasi_screen_obj_t *this);
int aasi_screen_obj_get_y(const aasi_screen_obj_t *this);
int aasi_screen_obj_get_x(const aasi_screen_obj_t *this);
int aasi_screen_obj_max_y(const aasi_screen_obj_t *this);
int aasi_screen_obj_max_x(const aasi_screen_obj_t *this);
int aasi_screen_obj_get_center(const aasi_screen_obj_t *this);
void aasi_screen_obj_hit(aasi_screen_obj_t *this);
bool aasi_screen_obj_is_collision(const aasi_screen_obj_t *this, const aasi_screen_obj_t *other);

// protected:
bool _aasi_screen_obj_init(aasi_screen_obj_t *this,
                           const aasi_screen_obj_ops_t *ops,
                           struct _aasi_game_t *game,
                           const char *shape,
                           int y,
                           int x);
void _aasi_screen_obj_move_absolute(aasi_screen_obj_t *this, int abs_y, int abs_x);
void _aasi_screen_obj_move_relative(aasi_screen_obj_t *this, int rel_y, int rel_x);
void _aasi_screen_obj_draw(aasi_screen_obj_t *this);
void _aasi_screen_obj_clear(aasi_screen_obj_t *this);
unsigned long _aasi_screen_obj_millis(const aasi_screen_obj_t *this);
bool _aasi_screen_obj_is_timeout(const aasi_screen_obj_t *this, unsigned long ts_start, unsigned long interval);
unsigned int _aasi_screen_obj_rand(const aasi_screen_obj_t *this);

#endif
