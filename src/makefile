IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wextra -Wshadow -Wcast-align -Wstrict-prototypes -Wstrict-overflow=2 -Wcast-qual -Wswitch-enum -Wconversion -Wunreachable-code -Wformat=2 -Wwrite-strings -Werror -Os -Winit-self -ggdb -march=native

ODIR=obj
LDIR =../lib

LIBS=

_DEPS = def.h log.h config.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = showiumize.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

showiumize: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
