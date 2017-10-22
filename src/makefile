IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wextra -Wshadow -Wcast-align -Wstrict-prototypes -Wstrict-overflow=2 -Wcast-qual -Wswitch-enum -Wconversion -Wunreachable-code -Wformat=2 -Wwrite-strings -Os -Winit-self -ggdb -march=native

ODIR=obj
LDIR =../lib

LIBS=-lcurl

_DEPS = curl_help.h definitions.h messages.h structs.h utils.h log.h config.h showrss.h premiumize.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = utils.o showiumize.o premiumize.o showrss.o config.o log.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

showiumize: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
