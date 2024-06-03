#include "molly.h"

static const int steps[] = { 1, 16, -16, -1, 17, 15, -17, -15 };
static const int jumps[] = { 18, 33, 31, 14, -18, -33, -31, -14 };

static void
mvaddf(struct gen *gen, int from, int to, int flag)
{
	gen->moves[gen->size++] = MV_MAKE(from, to, flag);
}

static void
mvadd(struct gen *gen, int from, int to)
{
	mvaddf(gen, from, to, 0);
}

static void
mvaddpawn(struct gen *gen, int from, int to, int stm)
{
	if (RELATIVE_PPRANK(from, stm)) {
		mvaddf(gen, from, to, MV_PROMON);
		mvaddf(gen, from, to, MV_PROMOB);
		mvaddf(gen, from, to, MV_PROMOR);
		mvaddf(gen, from, to, MV_PROMOQ);
	} else
		mvadd(gen, from, to);
}

static int
checker(const struct position *pos, int kng, int stm)
{
	int piece, from, to;
	int dir, i, chk = 0;

	stm ^= BOTH;
	for (i = 1; i < 16; i++) {
		from = pos->data[stm + i];

		if (from == 0)
			continue;

		piece = pos->board[from];

		if (pseudo_attack(from, kng, piece)) {
			if (PIECE_IS_SLIDER(piece)) {
				dir = direction(from, kng);
				for (to = from + dir; pos->board[to] == EMPTY;
				     to += dir)
					;

				if (to == kng)
					chk = chk ? -1 : from;
			} else
				chk = chk ? -1 : from;
		}
	}

	return chk;
}

static void
gevasions(const struct position *pos, struct gen *gen, int kng, int chk,
    int stm)
{
	int slider = PIECE_IS_SLIDER(pos->board[chk]);
	int dir = direction(kng, chk);
	int north = RELATIVE_NORTH(stm);
	int ep = pos->data[SQ_EP];
	int piece, from, to;
	int sq, i, obs;

	for (i = 1; i < 16; i++) {
		from = pos->data[stm + i];

		if (from == 0)
			continue;

		piece = pos->board[from];

		if (PIECE_TYPE(piece) == PAWN) {
			to = from + north;

			if (slider) {
				for (sq = kng + dir; sq != chk; sq += dir) {
					if (to == sq)
						mvaddpawn(gen, from, sq, stm);

					if (pos->board[to] == EMPTY &&
					    pos->board[to + north] == EMPTY &&
					    to + north == sq &&
					    RELATIVE_DPRANK(from, stm))
						mvaddf(gen, from, sq,
						    MV_DOUBLEPUSH);
				}
			}

			if (pseudo_attack(from, chk, piece))
				mvaddpawn(gen, from,
				    from + direction(from, chk), stm);

			if (ep - north == chk &&
			    PIECE_TYPE(pos->board[chk]) == PAWN &&
			    pseudo_attack(from, ep, piece))
				mvaddf(gen, from, ep, MV_ENPASSANT);
		} else {
			sq = slider ? kng : chk - dir;

			do {
				sq += dir;

				if (pseudo_attack(from, sq, piece)) {
					if (PIECE_IS_SLIDER(piece)) {
						obs = direction(from, sq);
						for (to = from + obs;
						     to != sq &&
						     pos->board[to] == EMPTY;
						     to += obs)
							;

						if (to == sq)
							mvadd(gen, from, sq);
					} else
						mvadd(gen, from, sq);
				}
			} while (sq != chk);
		}
	}
}

static void
gnormal(const struct position *pos, struct gen *gen, int stm)
{
	int north = RELATIVE_NORTH(stm);
	int ep = pos->data[SQ_EP];
	int piece, from, to;
	int i, j;

	for (i = 1; i < 16; i++) {
		from = pos->data[stm + i];

		if (from == 0)
			continue;

		piece = pos->board[from];

		switch (PIECE_TYPE(piece)) {
		case PAWN:
			to = from + north;

			if (PIECE_FLAG(pos->board[to + 1] ^ BOTH) == stm)
				mvaddpawn(gen, from, to + 1, stm);

			if (PIECE_FLAG(pos->board[to - 1] ^ BOTH) == stm)
				mvaddpawn(gen, from, to - 1, stm);

			if (pos->board[to] == EMPTY) {
				mvaddpawn(gen, from, to, stm);

				to += north;
				if (pos->board[to] == EMPTY &&
				    RELATIVE_DPRANK(from, stm))
					mvaddf(gen, from, to, MV_DOUBLEPUSH);
			}
			break;
		case KNIGHT:
			for (j = 0; j < 8; j++) {
				to = from + jumps[j];

				if (!(pos->board[to] & stm))
					mvadd(gen, from, to);
			}
			break;
		case BISHOP:
			for (j = 0; j < 4; j++) {
				to = from + steps[j + 4];

				for (; pos->board[to] == EMPTY;
				     to += steps[j + 4])
					mvadd(gen, from, to);

				if (!(pos->board[to] & stm))
					mvadd(gen, from, to);
			}
			break;
		case ROOK:
			for (j = 0; j < 4; j++) {
				to = from + steps[j];

				for (; pos->board[to] == EMPTY; to += steps[j])
					mvadd(gen, from, to);

				if (!(pos->board[to] & stm))
					mvadd(gen, from, to);
			}
			break;
		case QUEEN:
			for (j = 0; j < 8; j++) {
				to = from + steps[j];

				for (; pos->board[to] == EMPTY; to += steps[j])
					mvadd(gen, from, to);

				if (!(pos->board[to] & stm))
					mvadd(gen, from, to);
			}
			break;
		}
	}

	from = pos->data[stm];
	if (pos->data[from + 8] & pos->data[from + 3 + 8]) {
		if (pos->board[from + 1] == EMPTY &&
		    !attacked(pos, from + 1, stm ^ BOTH) &&
		    pos->board[from + 2] == EMPTY)
			mvaddf(gen, from, from + 2, MV_CASTLESH);
	}

	if (pos->data[from + 8] & pos->data[from - 4 + 8]) {
		if (pos->board[from - 1] == EMPTY &&
		    !attacked(pos, from - 1, stm ^ BOTH) &&
		    pos->board[from - 2] == EMPTY &&
		    pos->board[from - 3] == EMPTY)
			mvaddf(gen, from, from - 2, MV_CASTLELO);
	}

	if (ep) {
		from = ep - RELATIVE_NORTH(stm);

		if (pos->board[from + 1] == PAWN + stm)
			mvaddf(gen, from + 1, ep, MV_ENPASSANT);

		if (pos->board[from - 1] == PAWN + stm)
			mvaddf(gen, from - 1, ep, MV_ENPASSANT);
	}
}

void
genall(const struct position *pos, struct gen *gen)
{
	int stm = pos->data[SQ_STM];
	int kng = pos->data[stm];
	int chk = checker(pos, kng, stm);
	int to, i;

	gen->size = 0;

	for (i = 0; i < 8; i++) {
		to = kng + steps[i];

		if (!(pos->board[to] & stm))
			mvadd(gen, kng, to);
	}

	if (chk == 0)
		gnormal(pos, gen, stm);
	else if (chk > 0)
		gevasions(pos, gen, kng, chk, stm);
}
