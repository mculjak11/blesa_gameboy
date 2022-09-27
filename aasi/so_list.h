#ifndef _AASI_SO_LIST_H_
#define _AASI_SO_LIST_H_

#include <stdbool.h>

#define AASI_SO_LIST_SIZE 5
#define AASI_SO_LIST_FOR_EACH(lst, elem) \
	struct _aasi_screen_obj_t *elem; \
	for (int i = 0; \
	     (elem = aasi_so_list_get(lst, i)); \
	     ++i)

struct _aasi_screen_obj_t;

typedef struct _aasi_so_list_t {
	// private:
	struct _aasi_screen_obj_t *_elem[AASI_SO_LIST_SIZE];
	int _size;
} aasi_so_list_t;

void aasi_so_list_init(aasi_so_list_t *this);
bool aasi_so_list_add(aasi_so_list_t *this, struct _aasi_screen_obj_t *e);
int aasi_so_list_find(aasi_so_list_t *this, struct _aasi_screen_obj_t *e);
bool aasi_so_list_erase(aasi_so_list_t *this, struct _aasi_screen_obj_t *e);
void aasi_so_list_destroy(aasi_so_list_t *this);
struct _aasi_screen_obj_t* aasi_so_list_get(const aasi_so_list_t *this, int pos);
bool aasi_so_list_empty(const aasi_so_list_t *this);

#endif
