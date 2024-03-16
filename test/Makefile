.POSIX:
.SUFFIXES:
.force:
CC = cc
STD = gnu2x
NCOLOR = 0
DELIM = echo "~~"
ASAN = -fsanitize=address
UBSAN = -fsanitize=undefined
TSAN = -fsanitize=thread
CFLAGS = -std=$(STD) -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wcast-qual -Wvla -Werror -pedantic -O0 -g3 -DNCOLOR=$(NCOLOR)

check: test_arena test_spinlock test_msi

test_arena: arena
	@$(DELIM)
	@./arena
	@$(DELIM)

arena: arena.c .force
	$(CC) -o $@ $(CFLAGS) $<

test_spinlock: spinlock_tsan spinlock_asan
	@$(DELIM)
	@echo "[Under TSan]"
	@./spinlock_tsan
	@echo "[Under ASan/UBSan]"
	@./spinlock_asan
	@$(DELIM)

spinlock_tsan: spinlock.c .force
	$(CC) -o $@ $(CFLAGS) $< -lpthread

spinlock_asan: spinlock.c .force
	$(CC) -o $@ $(CFLAGS) $< -lpthread

test_msi: msi
	@$(DELIM)
	@./msi
	@$(DELIM)

msi: msi.c .force
	$(CC) -o $@ $(CFLAGS) $<
