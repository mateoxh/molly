#include <string.h>
#include "molly.h"

void
poscpy(struct position *dest, const struct position *src)
{
	memcpy(dest->board + SQ_A1, src->board + SQ_A1, 8);
	memcpy(dest->board + SQ_A2, src->board + SQ_A2, 8);
	memcpy(dest->board + SQ_A3, src->board + SQ_A3, 8);
	memcpy(dest->board + SQ_A4, src->board + SQ_A4, 8);
	memcpy(dest->board + SQ_A5, src->board + SQ_A5, 8);
	memcpy(dest->board + SQ_A6, src->board + SQ_A6, 8);
	memcpy(dest->board + SQ_A7, src->board + SQ_A7, 8);
	memcpy(dest->board + SQ_A8, src->board + SQ_A8, 8);

	memcpy(dest->data + SQ_A1, src->data + SQ_A1, 16);
	memcpy(dest->data + SQ_A2, src->data + SQ_A2, 16);
	memcpy(dest->data + SQ_A3, src->data + SQ_A3, 16);
	memcpy(dest->data + SQ_A4, src->data + SQ_A4, 16);
	memcpy(dest->data + SQ_A5, src->data + SQ_A5, 16);
	memcpy(dest->data + SQ_A6, src->data + SQ_A6, 16);
	memcpy(dest->data + SQ_A7, src->data + SQ_A7, 16);
	memcpy(dest->data + SQ_A8, src->data + SQ_A8, 16);

	memcpy(dest->data + BLACK, src->data + BLACK, 32);

	dest->data[SQ_STM] = src->data[SQ_STM];
	dest->data[SQ_EP] = src->data[SQ_EP];
}

int
legal(struct position *pos, const struct move *mv)
{
	int stm = pos->data[SQ_STM];
	int kng = pos->data[stm];
	int dir = direction(kng, mv->from);
	int to, atk;

	if (mv->from == kng) {
		pos->board[kng] = EMPTY;
		atk = attacked(pos, mv->to, stm ^ BOTH);
		pos->board[kng] = KING + stm;

		return !atk;
	} else if (mv->flag == MV_ENPASSANT) {
		if (dir == 1 || dir == -1) {
			for (to = kng + dir; pos->board[to] == EMPTY; to += dir)
				;

			if (to != mv->from && to != (mv->to ^ 16))
				return 1;

			to = mv->from + dir;
			if (to == (mv->to ^ 16))
				to += dir;

			for (; pos->board[to] == EMPTY; to += dir)
				;

			if (PIECE_FLAG(pos->board[to]) != (stm ^ BOTH))
				return 1;

			return !pseudo_attack(to, kng, pos->board[to]);
		}
	}

	if (dir == 0 || dir == direction(kng, mv->to))
		return 1;

	for (to = kng + dir; pos->board[to] == EMPTY; to += dir)
		;

	if (to != mv->from)
		return 1;

	for (to += dir; pos->board[to] == EMPTY; to += dir)
		;

	if (PIECE_FLAG(pos->board[to]) != (stm ^ BOTH))
		return 1;

	return !pseudo_attack(to, kng, pos->board[to]);
}

int
attacked(const struct position *pos, int sq, int stm)
{
	int from, piece;
	int to, dir, i;

	assert(SQ_OK(sq));

	for (i = 0; i < 16; i++) {
		from = pos->data[stm + i];

		if (from == 0)
			continue;

		piece = pos->board[from];

		assert(SQ_OK(from));
		assert(piece);

		if (pseudo_attack(from, sq, piece)) {
			if (!PIECE_IS_SLIDER(piece))
				return 1;

			dir = direction(from, sq);
			for (to = from + dir; to != sq && !pos->board[to]; to += dir)
				;

			if (to == sq)
				return 1;
		}
	}

	return 0;
}

int
checked(const struct position *pos)
{
	int stm = pos->data[SQ_STM];
	int kng = pos->data[stm];

	return attacked(pos, kng, stm ^ BOTH);
}

static void
movep(struct position *pos, int from, int to)
{
	pos->data[to] = pos->data[from];
	pos->board[to] = pos->board[from];
	pos->data[pos->data[to]] = to;

	pos->board[from] = EMPTY;
	pos->data[from] = EMPTY;
}

void
make(struct position *pos, const struct move *mv)
{
	int stm = pos->data[SQ_STM];

	assert(SQ_OK(mv->from));
	assert(SQ_OK(mv->to));

	pos->data[pos->data[mv->to]] = EMPTY;

	movep(pos, mv->from, mv->to);

	pos->data[SQ_EP] = EMPTY;
	pos->data[mv->from + 8] = EMPTY;
	pos->data[mv->to + 8] = EMPTY;

	switch (mv->flag) {
		case MV_DOUBLEPUSH:
			pos->data[SQ_EP] = mv->to ^ 16;
			break;
		case MV_ENPASSANT:
			pos->data[pos->data[mv->to ^ 16]] = EMPTY;
			pos->data[mv->to ^ 16]  = EMPTY;
			pos->board[mv->to ^ 16] = EMPTY;
			break;
		case MV_PROMON:
		case MV_PROMOB:
		case MV_PROMOR:
		case MV_PROMOQ:
			pos->board[mv->to] = mv->flag + stm;
			break;
		case MV_CASTLESH:
			movep(pos, mv->to + 1, mv->to - 1);
			break;
		case MV_CASTLELO:
			movep(pos, mv->to - 2, mv->to + 1);
			break;
	}

	pos->data[SQ_STM] ^= BOTH;
}
