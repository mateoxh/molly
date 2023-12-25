#include <stdio.h>
#include <string.h>
#include "molly.h"

void
sqtos(int sq, char *buf)
{
	buf[0] = 'a' + SQ_FILE(sq) - SQ_FILE(SQ_A1);
	buf[1] = '1' + SQ_RANK(sq) - SQ_RANK(SQ_A1);
	buf[2] = '\0';
}

void
mtos(int mv, char *buf)
{
	sqtos(MV_FROM(mv), buf);
	sqtos(MV_TO(mv), buf + 2);

	switch (MV_FLAG(mv)) {
		case MV_PROMON:
			buf[4] = 'n';
			break;
		case MV_PROMOB:
			buf[4] = 'b';
			break;
		case MV_PROMOR:
			buf[4] = 'r';
			break;
		case MV_PROMOQ:
			buf[4] = 'q';
			break;
		default:
			buf[4] = '\0';
			break;
	}

	buf[5] = '\0';
}

int
stosq(const char *s)
{
	int f = s[0] - 'a' + SQ_FILE(SQ_A1);
	int r = s[1] - '1' + SQ_RANK(SQ_A1);

	return f + r * 16;
}

int
starts_with(const char *s, const char *prefix)
{
	return strstr(s, prefix) == s;
}
