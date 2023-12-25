#include "molly.h"

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
make(struct position *pos, const struct move *mv, struct undo *u)
{
	int stm = pos->data[SQ_STM];

	assert(SQ_OK(mv->from));
	assert(SQ_OK(mv->to));

	/* for undo-ing */
	u->index = pos->data[mv->to];
	u->piece = PIECE_TYPE(pos->board[mv->to]);
	/* done */

	pos->data[pos->data[mv->to]] = EMPTY;
	movep(pos, mv->from, mv->to);

	/* for undo-ing */
	u->ep = pos->data[SQ_EP];
	u->cr_from = pos->data[mv->from + 8];
	u->cr_to = pos->data[mv->to + 8];
	/* done */

	pos->data[SQ_EP] = EMPTY;
	pos->data[mv->from + 8] = EMPTY;
	pos->data[mv->to + 8] = EMPTY;

	switch (mv->flag) {
		case MV_DOUBLEPUSH:
			pos->data[SQ_EP] = mv->to ^ 16;
			break;
		case MV_ENPASSANT:
			u->index = pos->data[mv->to^16];
			u->piece = PAWN;

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

void unmake(struct position *pos, const struct move *mv, struct undo *u)
{
	int stm = pos->data[SQ_STM] ^= BOTH;

	assert(SQ_OK(mv->from));
	assert(SQ_OK(mv->to));

	movep(pos, mv->to, mv->from);
	/* undo-ing */
	if (u->piece != EMPTY) {
		int to = mv->flag == MV_ENPASSANT ? (mv->to^16) : mv->to;
		pos->data[to] = u->index;
		pos->data[u->index] = to;
		pos->board[to] = u->piece + (stm ^ BOTH);
	}

	pos->data[SQ_EP] = u->ep;
	pos->data[mv->from + 8] = u->cr_from;
	pos->data[mv->to + 8] = u->cr_to;

	switch (mv->flag) {
		case MV_DOUBLEPUSH:
			break;
		case MV_ENPASSANT:
			break;
		case MV_PROMON:
		case MV_PROMOB:
		case MV_PROMOR:
		case MV_PROMOQ:
			pos->board[mv->from] = PAWN + stm;
			break;
		case MV_CASTLESH:
			movep(pos, mv->to - 1, mv->to + 1);
			break;
		case MV_CASTLELO:
			movep(pos, mv->to + 1, mv->to - 2);
			break;
	}
}
