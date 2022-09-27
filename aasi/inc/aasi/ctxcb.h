#ifndef _AASI_CTXCB_H_
#define _AASI_CTXCB_H_

typedef void (*aasi_ctxcb_cb_t)(void *priv);

typedef struct _aasi_ctxcb_t {
	aasi_ctxcb_cb_t cb;
	void *priv;
} aasi_ctxcb_t;

void aasi_ctxcb_init(aasi_ctxcb_t *this);
void aasi_ctxcb_set(aasi_ctxcb_t *this, aasi_ctxcb_cb_t cb, void *priv);
void aasi_ctxcb_call(const aasi_ctxcb_t *this);

#endif
