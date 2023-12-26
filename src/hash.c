#include <stdlib.h>
#include <time.h>
#include "molly.h"

static Hash zkeys[2][6][128];
static Hash zkeys_castle[4];
static Hash zkeys_stm[2];
static Hash zkeys_enpassant[8];

static Hash
prng()
{
	return (Hash)rand() | ((Hash)rand()) << 32;
}

Hash
hash(struct position *pos)
{
	Hash h = 0;
	int stm = pos->data[SQ_STM];
	int from, piece;

	for (int i = 0; i < 32; i++) {
		int from = pos->data[BLACK + i];
		if (from != EMPTY) {
			piece = PIECE_TYPE(pos->board[from]);
			h ^= zkeys[i < 16][piece - 1][from - SQ_A1];
		}
	}

	from = pos->data[WHITE];
	if (pos->data[from + 8] & pos->data[from + 3 + 8]) h ^= zkeys_castle[0];
	if (pos->data[from + 8] & pos->data[from - 4 + 8]) h ^= zkeys_castle[1];

	from = pos->data[BLACK];
	if (pos->data[from + 8] & pos->data[from + 3 + 8]) h ^= zkeys_castle[2];
	if (pos->data[from + 8] & pos->data[from - 4 + 8]) h ^= zkeys_castle[3];

	h ^= zkeys_stm[stm == WHITE];

	from = pos->data[SQ_EP];
	if (from)
		h ^= zkeys_enpassant[SQ_FILE(from) - SQ_FILE(SQ_A1)];

	return h;
}

void
hash_init()
{
	int i, j, k;
	srand(time(NULL));

	for (i = 0; i < 2; i++)
		for (j = 0; j < 6; j++)
			for (k = 0; k < 128; k++)
				zkeys[i][j][k] = prng();

	for (i = 0; i < 4; i++)
		zkeys_castle[i] = prng();

	for (i = 0; i < 2; i++)
		zkeys_stm[i] = prng();

	for (i = 0; i < 8; i++)
		zkeys_enpassant[i] = prng();
}
