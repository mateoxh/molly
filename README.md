## Overview
Molly is a chess program that calculates perft counts, the total number of possible positions that can be reached within a given depth.

## How to use it
The available commands are described inside the program, to see them just type "help".

### Options
    -h <N>    set the size of the hash table to N MB
    -v        verbose mode, display additional information during execution
    
### Example
Sample output in verbose mode on the well known [Kiwipete](https://www.chessprogramming.org/Perft_Results#Position_2) position:

    $ ./molly -h 256 -v
    Hash Table initialized to 256 MB
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 
    perft 6
    perft 1 (48) in 0.00 s
    perft 2 (2039) in 0.00 s
    perft 3 (97862) in 0.01 s
    perft 4 (4085603) in 0.09 s
    perft 5 (193690690) in 0.78 s
    perft 6 (8031647685) in 27.28 s
    quit


## Installing
After downloading the sources, to build the program run these commands

    cd molly/src/
    make build
