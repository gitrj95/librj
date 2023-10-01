.POSIX:
.SUFFIXES:
.FORCE:

CC = cc
STD = gnu2x # typeof operators not in gcc/clang yet for c23
NCOLOR = 0
DELIM = echo "~~"
ASAN = -fsanitize=address
UBSAN = -fsanitize=undefined
TSAN = -fsanitize=thread
CFLAGS = -std=$(STD) -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wcast-qual -Wvla -Werror -O0 -fpie -g3 -DNCOLOR=$(NCOLOR)
LDFLAGS = -Wl,-pie

check: test_arena test_spscqueue test_spinlock test_msi

test_arena: arena
	@$(DELIM)
	@./arena
	@$(DELIM)

arena: arena.c .FORCE
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS)

test_spscqueue: spscqueue_tsan spscqueue_asan
	@$(DELIM)
	@echo "[Under TSan]"
	@./spscqueue_tsan
	@echo "[Under ASan/UBSan]"
	@./spscqueue_asan
	@$(DELIM)

spscqueue_tsan: spscqueue.c .FORCE
	$(CC) -o $@ $(CFLAGS) $(TSAN) $< $(LDFLAGS) -lpthread

spscqueue_asan: spscqueue.c .FORCE
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS) -lpthread

test_spinlock: spinlock_tsan spinlock_asan
	@$(DELIM)
	@echo "[Under TSan]"
	@./spinlock_tsan
	@echo "[Under ASan/UBSan]"
	@./spinlock_asan
	@$(DELIM)

spinlock_tsan: spinlock.c .FORCE
	$(CC) -o $@ $(CFLAGS) $(TSAN) $< $(LDFLAGS) -lpthread

spinlock_asan: spinlock.c .FORCE
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS) -lpthread

test_msi: msi
	@$(DELIM)
	@./msi
	@$(DELIM)

msi: msi.c .FORCE
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS)