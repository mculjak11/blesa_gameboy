#include <string.h>

#include "screen_obj.h"
#include "so_list.h"


void aasi_so_list_init(aasi_so_list_t *this) {
	memset(this->_elem, 0, sizeof(this->_elem));
	this->_size = 0;
}

bool aasi_so_list_add(aasi_so_list_t *this, aasi_screen_obj_t *e) {
	if (!e) {
		return false;
	} else if (this->_size >= AASI_SO_LIST_SIZE) {
		aasi_screen_obj_delete(e);
		return false;
	}

	this->_elem[this->_size] = e;
	this->_size++;
	return true;
}

int aasi_so_list_find(aasi_so_list_t *this, aasi_screen_obj_t *e) {
	for (int i = 0; i < this->_size; ++i) {
		if (this->_elem[i] == e) {
			return i;
		}
	}
	return -1;
}

bool aasi_so_list_erase(aasi_so_list_t *this, aasi_screen_obj_t *e) {
	int pos = aasi_so_list_find(this, e);
	if (pos >= 0) {
		const int elems_after_pos = this->_size - pos - 1;
		aasi_screen_obj_delete(this->_elem[pos]);
		memmove(this->_elem + pos, this->_elem + pos + 1, sizeof(aasi_screen_obj_t*) * elems_after_pos);
		this->_size--;
		return true;
	} else {
		return false;
	}
}

void aasi_so_list_destroy(aasi_so_list_t *this) {
	while (this->_size) {
		aasi_so_list_erase(this, *this->_elem);
	}
}

aasi_screen_obj_t* aasi_so_list_get(const aasi_so_list_t *this, int pos) {
	if (pos < 0 || pos >= this->_size) {
		return NULL;
	}
	return this->_elem[pos];
}

bool aasi_so_list_empty(const aasi_so_list_t *this) {
	return this->_size == 0;
}
