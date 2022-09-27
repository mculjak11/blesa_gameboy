#ifndef _AASI_DISPLAY_H_
#define _AASI_DISPLAY_H_

#include <stdbool.h>

struct _aasi_display_t;
typedef struct _aasi_display_t aasi_display_t;

typedef struct _aasi_display_ops_t {
	void (*start)(aasi_display_t *this);
	void (*destroy)(aasi_display_t *this);
	void (*mvclr)(aasi_display_t *this, void **obj, int y, int x, const char *s);
	void (*mvputs)(aasi_display_t *this, void **obj, int y, int x, const char *s);
	void (*objdel)(aasi_display_t *this, void **obj);
} aasi_display_ops_t;

struct _aasi_display_t {
	// private:
	const aasi_display_ops_t *_ops;
	int _width;
	int _height;
};

bool aasi_display_init(aasi_display_t *this, const aasi_display_ops_t *ops, int width, int height);
void aasi_display_destroy(aasi_display_t *this);
void aasi_display_start(aasi_display_t *this);
void aasi_display_mvclr(aasi_display_t *this, void **obj, int y, int x, const char *s);
void aasi_display_mvputs(aasi_display_t *this, void **obj, int y, int x, const char *s);
void aasi_display_objdel(aasi_display_t *this, void **obj);
int aasi_display_width(const aasi_display_t *this);
int aasi_display_height(const aasi_display_t *this);

#endif
