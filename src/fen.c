#include <string.h>

#include "molly.h"

static int
atop(char c)
{
	static const char *piecestr = "?????????????????kpnbrq??????????KPNBRQ";
	return strchr(piecestr, c) - piecestr;
}

static const char *
skipws(const char *s)
{
	while (*s && *s == ' ')
		s++;

	return s;
}

void
clear(struct position *pos)
{
	int i;

	for (i = 0; i < BOARDSIZ; i++) {
		pos->board[i] = SQ_OK(i) ? EMPTY : BOTH;
		pos->data[i] = EMPTY;
	}
}

static void
placep(struct position *pos, char c, int sq)
{
	int piece = atop(c);
	int index = PIECE_FLAG(piece);
	int i;

	if (PIECE_TYPE(piece) != KING) {
		for (i = 1; i < 16; i++) {
			index++;

			if (pos->data[index] == EMPTY)
				break;
		}
	}

	pos->board[sq] = piece;
	pos->data[sq] = index;
	pos->data[index] = sq;
}

static void
setstm(struct position *pos, char stm)
{
	pos->data[SQ_STM] = stm == 'w' ? WHITE : BLACK;
}

static void
setcr(struct position *pos, int ks, int rs)
{
	pos->data[ks + 8] = 1;
	pos->data[rs + 8] = 1;
}

static void
setep(struct position *pos, int ep)
{
	pos->data[SQ_EP] = ep;
}

void
setup(struct position *pos, const char *fen)
{
	int sq = SQ_A8;
	char c;

	clear(pos);

	fen = skipws(fen);
	for (; (c = *fen) && c != ' '; fen++) {
		switch (c) {
		case '/':
			sq -= 24;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			sq += c - '0';
			break;
		case 'k':
		case 'p':
		case 'n':
		case 'b':
		case 'r':
		case 'q':
		case 'K':
		case 'P':
		case 'N':
		case 'B':
		case 'R':
		case 'Q':
			placep(pos, c, sq++);
			break;
		}
	}

	fen = skipws(fen);
	if ((c = *fen)) {
		fen++;

		switch (c) {
		case 'w':
		case 'b':
			setstm(pos, c);
			break;
		}
	}

	fen = skipws(fen);
	for (; (c = *fen) && c != ' '; fen++) {
		switch (c) {
		case 'K':
			setcr(pos, SQ_A1 + 'e' - 'a', SQ_A1 + 'h' - 'a');
			break;
		case 'Q':
			setcr(pos, SQ_A1 + 'e' - 'a', SQ_A1);
			break;
		case 'k':
			setcr(pos, SQ_A8 + 'e' - 'a', SQ_A8 + 'h' - 'a');
			break;
		case 'q':
			setcr(pos, SQ_A8 + 'e' - 'a', SQ_A8);
			break;
		}
	}

	fen = skipws(fen);
	if ((c = *fen) && c != '-')
		setep(pos, stosq(fen));
}
