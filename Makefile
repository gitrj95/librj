.POSIX:
.SUFFIXES:
.force:
CC = cc
STD = gnu2x
NCOLOR = 0
CFLAGS = -std=$(STD) -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wcast-qual -Wvla -Werror -pedantic -flto
ARFLAGS = -rcs

all: bin/librj.a

check: .force
	@cd test && $(MAKE) NCOLOR=$(NCOLOR)

clean: .force
	cd bin && rm -rf *
	cd bin && touch .gitkeep

bin/librj.a: bin/posix_arena.o
	ar $(ARFLAGS) $@ bin/posix_arena.o

bin/posix_arena.o: posix_arena.c arena.h
	$(CC) -o $@ $(CFLAGS) -c $<
