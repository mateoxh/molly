#include "molly.h"

int
legal(struct position *pos, int mv)
{
	int stm = pos->data[SQ_STM];
	int kng = pos->data[stm];
	int dir = direction(kng, MV_FROM(mv));
	int to, atk;

	if (MV_FROM(mv) == kng) {
		pos->board[kng] = EMPTY;
		atk = attacked(pos, MV_TO(mv), stm ^ BOTH);
		pos->board[kng] = KING + stm;

		return !atk;
	} else if (MV_FLAG(mv) == MV_ENPASSANT) {
		if (dir == 1 || dir == -1) {
			for (to = kng + dir; pos->board[to] == EMPTY; to += dir)
				;

			if (to != MV_FROM(mv) && to != (MV_TO(mv) ^ 16))
				return 1;

			to = MV_FROM(mv) + dir;
			if (to == (MV_TO(mv) ^ 16))
				to += dir;

			for (; pos->board[to] == EMPTY; to += dir)
				;

			if (PIECE_FLAG(pos->board[to]) != (stm ^ BOTH))
				return 1;

			return !pseudo_attack(to, kng, pos->board[to]);
		}
	}

	if (dir == 0 || dir == direction(kng, MV_TO(mv)))
		return 1;

	for (to = kng + dir; pos->board[to] == EMPTY; to += dir)
		;

	if (to != MV_FROM(mv))
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
make(struct position *pos, int mv, struct undo *u)
{
	int stm = pos->data[SQ_STM];

	assert(SQ_OK(MV_FROM(mv)));
	assert(SQ_OK(MV_TO(mv)));

	/* for undo-ing */
	u->index = pos->data[MV_TO(mv)];
	u->piece = PIECE_TYPE(pos->board[MV_TO(mv)]);
	/* done */

	pos->data[pos->data[MV_TO(mv)]] = EMPTY;
	movep(pos, MV_FROM(mv), MV_TO(mv));

	/* for undo-ing */
	u->ep = pos->data[SQ_EP];
	u->cr_from = pos->data[MV_FROM(mv) + 8];
	u->cr_to = pos->data[MV_TO(mv) + 8];
	/* done */

	pos->data[SQ_EP] = EMPTY;
	pos->data[MV_FROM(mv) + 8] = EMPTY;
	pos->data[MV_TO(mv) + 8] = EMPTY;

	switch (MV_FLAG(mv)) {
		case MV_DOUBLEPUSH:
			pos->data[SQ_EP] = MV_TO(mv) ^ 16;
			break;
		case MV_ENPASSANT:
			u->index = pos->data[MV_TO(mv)^16];
			u->piece = PAWN;

			pos->data[pos->data[MV_TO(mv) ^ 16]] = EMPTY;
			pos->data[MV_TO(mv) ^ 16]  = EMPTY;
			pos->board[MV_TO(mv) ^ 16] = EMPTY;
			break;
		case MV_PROMON:
		case MV_PROMOB:
		case MV_PROMOR:
		case MV_PROMOQ:
			pos->board[MV_TO(mv)] = MV_FLAG(mv) + stm;
			break;
		case MV_CASTLESH:
			movep(pos, MV_TO(mv) + 1, MV_TO(mv) - 1);
			break;
		case MV_CASTLELO:
			movep(pos, MV_TO(mv) - 2, MV_TO(mv) + 1);
			break;
	}

	pos->data[SQ_STM] ^= BOTH;
}

void unmake(struct position *pos, int mv, struct undo *u)
{
	int stm = pos->data[SQ_STM] ^= BOTH;

	assert(SQ_OK(MV_FROM(mv)));
	assert(SQ_OK(MV_TO(mv)));

	movep(pos, MV_TO(mv), MV_FROM(mv));
	/* undo-ing */
	if (u->piece != EMPTY) {
		int to = MV_FLAG(mv) == MV_ENPASSANT ? (MV_TO(mv)^16) : MV_TO(mv);
		pos->data[to] = u->index;
		pos->data[u->index] = to;
		pos->board[to] = u->piece + (stm ^ BOTH);
	}

	pos->data[SQ_EP] = u->ep;
	pos->data[MV_FROM(mv) + 8] = u->cr_from;
	pos->data[MV_TO(mv) + 8] = u->cr_to;

	switch (MV_FLAG(mv)) {
		case MV_PROMON:
		case MV_PROMOB:
		case MV_PROMOR:
		case MV_PROMOQ:
			pos->board[MV_FROM(mv)] = PAWN + stm;
			break;
		case MV_CASTLESH:
			movep(pos, MV_TO(mv) - 1, MV_TO(mv) + 1);
			break;
		case MV_CASTLELO:
			movep(pos, MV_TO(mv) + 1, MV_TO(mv) - 2);
			break;
	}
}
