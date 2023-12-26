#include <stdlib.h>
#include <string.h>
#include "molly.h"

/* 8 MB */
#define MEMORY (8 * 1024 * 1024)

static struct ttentry *tt;
static size_t N = MEMORY / sizeof(struct ttentry);

void
tt_init()
{
	tt = (struct ttentry *)malloc(MEMORY);
	memset(tt, 0, MEMORY);
}

void
put(Hash key, long nodes, int depth)
{
	struct ttentry *e = get(key);

	e->key = key;
	e->nodes = nodes;
	e->depth = depth;
}

struct ttentry *get(Hash h)
{
	return &tt[h % N];
}
