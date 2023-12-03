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
LDFLAGS = -Wl,-pie

check: test_arena test_spscqueue test_spinlock test_msi test_arraysize

test_arena: arena
	@$(DELIM)
	@./arena
	@$(DELIM)

arena: arena.c .force
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS)

test_spscqueue: spscqueue_tsan spscqueue_asan
	@$(DELIM)
	@echo "[Under TSan]"
	@./spscqueue_tsan
	@echo "[Under ASan/UBSan]"
	@./spscqueue_asan
	@$(DELIM)

spscqueue_tsan: spscqueue.c .force
	$(CC) -o $@ $(CFLAGS) $(TSAN) $< $(LDFLAGS) -lpthread

spscqueue_asan: spscqueue.c .force
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS) -lpthread

test_spinlock: spinlock_tsan spinlock_asan
	@$(DELIM)
	@echo "[Under TSan]"
	@./spinlock_tsan
	@echo "[Under ASan/UBSan]"
	@./spinlock_asan
	@$(DELIM)

spinlock_tsan: spinlock.c .force
	$(CC) -o $@ $(CFLAGS) $(TSAN) $< $(LDFLAGS) -lpthread

spinlock_asan: spinlock.c .force
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS) -lpthread

test_msi: msi
	@$(DELIM)
	@./msi
	@$(DELIM)

msi: msi.c .force
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS)

test_arraysize: arraysize
	@$(DELIM)
	@./arraysize
	@$(DELIM)

arraysize: arraysize.c .force
	$(CC) -o $@ $(CFLAGS) $(ASAN) $(UBSAN) $< $(LDFLAGS)
