#include <aasi/display.h>

bool aasi_display_init(aasi_display_t *this, const aasi_display_ops_t *ops, int width, int height) {
	if (!this || !ops || !ops->mvputs || width < 8 || height < 3) {
		return false;
	}
	this->_ops = ops;
	this->_width = width;
	this->_height = height;
	return true;
}

void aasi_display_destroy(aasi_display_t *this) {
	if (this && this->_ops && this->_ops->destroy) {
		this->_ops->destroy(this);
	}
}

void aasi_display_start(aasi_display_t *this) {
	if (this && this->_ops && this->_ops->start) {
		this->_ops->start(this);
	}
}

void aasi_display_mvclr(aasi_display_t *this, void **obj, int y, int x, const char *s) {
	if (this && this->_ops->mvclr) {
		this->_ops->mvclr(this, obj, y, x, s);
	}
}

void aasi_display_mvputs(aasi_display_t *this, void **obj, int y, int x, const char *s) {
	if (this && this->_ops) {
		this->_ops->mvputs(this, obj, y, x, s);
	}
}

void aasi_display_objdel(aasi_display_t *this, void **obj) {
	if (this && this->_ops->objdel) {
		this->_ops->objdel(this, obj);
	}
}

int aasi_display_width(const aasi_display_t *this) {
	return this->_width;
}

int aasi_display_height(const aasi_display_t *this) {
	return this->_height;
}

