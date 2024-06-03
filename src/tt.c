#include <stdlib.h>

#include "molly.h"

static struct ttentry *tt;
static size_t len;

void
tt_init(size_t mb)
{
	len = (mb * 1024 * 1024) / sizeof(struct ttentry);
	tt = calloc(len, sizeof(struct ttentry));
}

void
put(Hash key, long nodes, int depth)
{
	struct ttentry *e = get(key);

	e->key = key;
	e->nodes = nodes;
	e->depth = depth;
}

struct ttentry *
get(Hash h)
{
	return &tt[h % len];
}
