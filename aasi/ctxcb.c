#include <stddef.h>
#include <aasi/ctxcb.h>

void aasi_ctxcb_init(aasi_ctxcb_t *this) {
	aasi_ctxcb_set(this, NULL, NULL);
}

void aasi_ctxcb_set(aasi_ctxcb_t *this, aasi_ctxcb_cb_t cb, void *priv) {
	this->cb = cb;
	this->priv = priv;
}

void aasi_ctxcb_call(const aasi_ctxcb_t *this) {
	if (this->cb) {
		this->cb(this->priv);
	}
}
