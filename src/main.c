#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "molly.h"

#define AUTHOR   "Mateo Gjika"
#define PROGNAME "Molly"
#define VERSION  "0.2"

#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"
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
	int depth, verbose = 0, tt_size = DEFAULT_TT_SIZE;
	int opt;

	while ((opt = getopt(argc, argv, "vh:")) != -1) {
		switch (opt) {
			case 'h':
				tt_size = atoi(optarg);
				break;
			case 'v':
				verbose = 1;
				break;
		}
	}

	vector_init();
	hash_init();
	tt_init(tt_size);

	if (verbose)
		printf("Hash Table initialized to %d MB\n", tt_size);

	while (fgets(line, sizeof(line), stdin)) {
		if (starts_with(line, "position")) {
			const char *p = strchr(line, ' ');
			if (p != NULL && starts_with(p + 1, "fen"))
				setup(pos, line + strlen("position fen"));
			else
				setup(pos, DEFAULT_FEN);
		} else if (starts_with(line, "perft")) {
			sscanf(line + strlen("perft"), "%d", &depth);
			if (verbose)
				for (int d = 1; d <= depth; d++) {
					clock_t start = clock();
					long nodes = perft(pos, d);
					double sec = (double)(clock() - start) / CLOCKS_PER_SEC;
					printf("perft %d (%ld) in %.2f s\n", d, nodes, sec);
				}
			else
				printf("%ld\n", perft(pos, depth));
			fflush(stdout);
		} else if (starts_with(line, "divide")) {
			sscanf(line + strlen("divide"), "%d", &depth);
			divide(pos, depth);
		} else if (starts_with(line, "help")) {
			printf("%s, %s version %s\n", AUTHOR, PROGNAME, VERSION);
			printf("commands available:\n");
			printf("  position [startpos|fen <FEN>] - set up the position described in FEN on the internal\n");
			printf("                                  board for the standard start position startpos can be passed\n");
			printf("  perft <DEPTH>                 - start calculating Perft on the current position up to DEPTH\n");
			printf("  divide <DEPTH>                - run Perft for each one of the moves in the current position\n");
			printf("  quit                          - quit the program\n");
			printf("  help                          - print this message\n");
		} else if (starts_with(line, "quit")) {
			exit(0);
		} else {
			printf("command not recognized\n");
		}
	}
}
