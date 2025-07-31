.POSIX:
.SUFFIXES:.c .o

OBJ=msi.o

lib/librj.a: $(OBJ)
	$(AR) $(ARFLAGS) $@ $(OBJ)
