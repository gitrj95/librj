.POSIX:
.SUFFIXES:
.FORCE:

CC = cc
STD = gnu2x
NCOLOR = 0
CFLAGS = -std=$(STD) -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wcast-qual -Wvla -Werror -pedantic -O3 -DNDEBUG -flto -march=native
ARFLAGS = -rcs

all: bin/librj.a

tools: .FORCE
	ln -sf $(PWD)/compile_flags.txt $(HOME)/compile_flags.txt
	ln -sf $(PWD)/.clang-format $(HOME)/.clang-format

check: .FORCE
	@cd test && $(MAKE) NCOLOR=$(NCOLOR)

clean: .FORCE
	cd bin && rm -rf *
	cd bin && touch .gitkeep

bin/librj.a: bin/posix_arena.o bin/spscqueue.o
	ar $(ARFLAGS) $@ bin/posix_arena.o bin/spscqueue.o

bin/posix_arena.o: posix_arena.c arena.h ssize.h
	$(CC) -o $@ $(CFLAGS) -c $<

bin/spscqueue.o: spscqueue.c spscqueue.h ssize.h
	$(CC) -o $@ $(CFLAGS) -c $<
