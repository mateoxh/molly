CC= gcc

SRC= $(wildcard *.c)

WARN= -Wall -Wextra -pedantic
DEBG= -g -fno-omit-frame-pointer -fsanitize=address -O1
OPT=  -DNDEBUG -O3 -march=native -flto
BIN=  molly

all:
	$(CC) $(WARN) $(DEBG) $(SRC) -o $(BIN)

build:
	$(CC) $(WARN) $(OPT) $(SRC) -o $(BIN)

clean:
	@rm -f $(BIN)
