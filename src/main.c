#include <stdio.h>
#include <string.h>
#include "molly.h"

#define AUTHOR   "Mateo Gjika"
#define PROGNAME "Molly"
#define VERSION  "0.1"

#define POSSTACKSIZ 512

static struct position pstk[POSSTACKSIZ], *pos = pstk;

static unsigned long
perft(int d)
{
	unsigned long nodes = 0;
	struct gen gen;
	int i;

	genall(pos, &gen);

	if (d < 1)
		return 1;
	else if (d == 1) {
		for (i = 0; i < gen.size; i++) {
			if (legal(pos, gen.moves + i))
				nodes++;
		}
	} else {
		for (i = 0; i < gen.size; i++) {
			if (!legal(pos, gen.moves + i))
				continue;

			poscpy(pos + 1, pos);
			pos++;

			make(pos, gen.moves + i);
			nodes += perft(d - 1);

			pos--;
		}
	}

	return nodes;
}

static void
divide(int d)
{
	unsigned long nodes = 0, tnodes = 0;
	struct gen gen;
	char buf[6];
	int i;

	genall(pos, &gen);

	for (i = 0; i < gen.size; i++) {
		if (!legal(pos, gen.moves + i))
			continue;

		poscpy(pos + 1, pos);
		pos++;

		make(pos, gen.moves + i);
		mtos(gen.moves + i, buf);

		nodes = perft(d - 1);
		tnodes += nodes;
		printf("%s: %ld\n", buf, nodes);

		pos--;
	}

	printf("Total Nodes: %ld\n", tnodes);
}

int
main(int argc, char *argv[])
{
	char line[255];
	int depth, i;

	vector_init();

	for (i = 0; i < POSSTACKSIZ; i++)
		clear(pstk + i);

	if (argc > 1) {
		setup(pos, argv[1]);

		if (argc > 2) {
			sscanf(argv[2], "%d", &depth);

			if (depth > 0)
				divide(depth);
		}
	} else {
		while (fgets(line, 255, stdin)) {
			if (starts_with(line, "position"))
				setup(pos, line + strlen("position fen "));
			else if (starts_with(line, "perft")) {
				sscanf(line + strlen("perft "), "%d", &depth);
				printf("%ld\n", perft(depth));
				fflush(stdout);
			} else if (starts_with(line, "help")) {
				printf("%s, %s versione %s\n", AUTHOR, PROGNAME, VERSION);
			}
		}
	}

	return 0;
}
