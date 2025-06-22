.POSIX:
.SUFFIXES:.c .o

OBJ=linalloc.o msi.o

lib/librj.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)
