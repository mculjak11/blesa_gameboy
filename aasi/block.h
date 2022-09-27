#ifndef _AASI_BLOCK_H_
#define _AASI_BLOCK_H_

struct _aasi_block_t;
typedef struct _aasi_block_t aasi_block_t;
struct _aasi_game_t;

aasi_block_t *aasi_block_new(struct _aasi_game_t *game);
void aasi_block_delete(aasi_block_t *this);

#endif
