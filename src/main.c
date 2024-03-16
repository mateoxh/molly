#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "molly.h"

#define AUTHOR   "Mateo Gjika"
#define PROGNAME "Molly"
#define VERSION  "0.2"

#define DEFAULT_TT_SIZE 16

static long
perft(struct position *pos, int d)
{
	long nodes = 0;
	struct gen gen;
	int i;

	if (d < 1)
		return 1;

	genall(pos, &gen);
	for (i = 0; i < gen.size; i++) {
		if (!legal(pos, gen.moves[i]))
			continue;

		if (d == 1) {
			nodes++;
		} else {
			struct undo u;
			long result;

			make(pos, gen.moves[i], &u);

			Hash h = hash(pos);
			struct ttentry *e = get(h);

			if (e->key == h && e->depth == d - 1) {
				result = e->nodes;
			} else {
				result = perft(pos, d - 1);
				put(h, result, d - 1);
			}

			nodes += result;
			unmake(pos, gen.moves[i], &u);
		}
	}

	return nodes;
}

static void
divide(struct position *pos, int d)
{
	unsigned long nodes = 0, tnodes = 0;
	struct gen gen;
	char buf[6];
	int i;

	genall(pos, &gen);

	for (i = 0; i < gen.size; i++) {
		if (!legal(pos, gen.moves[i]))
			continue;

		struct undo u;
		make(pos, gen.moves[i], &u);
		mtos(gen.moves[i], buf);

		nodes = perft(pos, d - 1);
		tnodes += nodes;
		printf("%s: %ld\n", buf, nodes);
		unmake(pos, gen.moves[i], &u);
	}

	printf("Total Nodes: %ld\n", tnodes);
}

int
main(int argc, char *argv[])
{
	char line[255];
	struct position pos[1];
	int depth, tt_size = DEFAULT_TT_SIZE;
	int opt;

	while ((opt = getopt(argc, argv, "h:")) != -1) {
		switch (opt) {
			case 'h':
				tt_size = atoi(optarg);
				break;
		}
	}

	vector_init();
	hash_init();
	tt_init(tt_size);

	while (fgets(line, sizeof(line), stdin)) {
		if (starts_with(line, "position")) {
			setup(pos, line + strlen("position fen"));
		} else if (starts_with(line, "perft")) {
			sscanf(line + strlen("perft"), "%d", &depth);
			printf("%ld\n", perft(pos, depth));
			fflush(stdout);
		} else if (starts_with(line, "divide")) {
			sscanf(line + strlen("divide"), "%d", &depth);
			divide(pos, depth);
		} else if (starts_with(line, "info")) {
			printf("%s, %s versione %s\n", AUTHOR, PROGNAME, VERSION);
		}
	}
}
