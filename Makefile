#MakeFile
CC = gcc
CSTD = -std=c17
CFLAGS = -Wall -Werror -Wextra -lm -O3

all:
	$(CC) nuke.c -o nuke $(CSTD) $(CFLAGS)
	$(CC) map_generator.c -o map_generator $(CSTD) $(CFLAGS)

clean:
	rm nuke
	rm map_generator
 