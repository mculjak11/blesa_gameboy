#ifndef _OOC_H_
#define _OOC_H_

#include <stdlib.h>

// EXPL: why macro instead of static inline?
#define NEW(T) ((T*)malloc(sizeof(T)))

#define NEW_INIT(T, initf, ...) ({ \
	T *this = NEW(T); \
	if (!initf(this, __VA_ARGS__)) { \
		free(this); \
		this = NULL; \
	} \
	this; \
})

#endif
