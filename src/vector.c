#include "molly.h"

#define VECDELTA 119
#define VECSIZ	 (VECDELTA * 2 + 1)

static int dirvec[VECSIZ];
static long psattvec[VECSIZ];

static long
bitmap(int piece)
{
	return 1L << piece;
}

static long
wbmap(int piece)
{
	return bitmap(piece + WHITE) | bitmap(piece + BLACK);
}

void
vector_init(void)
{
	static const int steps[] = { 1, 16, -16, -1, 17, 15, -17, -15 };
	static const int jumps[] = { 18, 33, 31, 14, -18, -33, -31, -14 };

	int i, j;

	for (i = 0; i < 8; i++)
		for (j = 1; j < 8; j++)
			dirvec[VECDELTA + steps[i] * j] = steps[i];

	psattvec[VECDELTA + 15] = psattvec[VECDELTA + 17] = bitmap(
	    PAWN + WHITE);
	psattvec[VECDELTA - 15] = psattvec[VECDELTA - 17] = bitmap(
	    PAWN + BLACK);

	for (i = 0; i < 8; i++) {
		psattvec[VECDELTA + steps[i]] |= wbmap(KING);
		psattvec[VECDELTA + jumps[i]] |= wbmap(KNIGHT);

		for (j = 1; j < 8; j++) {
			psattvec[VECDELTA + steps[i] * j] |= wbmap(QUEEN);
			psattvec[VECDELTA + steps[i] * j] |= i < 4 ?
			    wbmap(ROOK) :
			    wbmap(BISHOP);
		}
	}
}

int
direction(int from, int to)
{
	assert(SQ_OK(from));
	assert(SQ_OK(to));

	return dirvec[VECDELTA + to - from];
}

int
pseudo_attack(int from, int to, int piece)
{
	assert(SQ_OK(from));
	assert(SQ_OK(to));

	return (psattvec[VECDELTA + to - from] & bitmap(piece)) != 0;
}
