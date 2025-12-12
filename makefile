CC = gcc
CFLAGS = -O3 -std=c11 -Wall -Wextra -Wshadow -Wconversion

SRC = src/rdt_core.c src/rdt_prng.c src/rdt_drbg.c
HDR = src/rdt_core.h src/rdt.h

PRNG_OBJ = rdt_core.o rdt_prng.o
DRBG_OBJ = rdt_core.o rdt_drbg.o

all: rdt_prng rdt_drbg

rdt_prng: $(PRNG_OBJ)
	$(CC) $(CFLAGS) -o rdt_prng $(PRNG_OBJ)

rdt_drbg: $(DRBG_OBJ)
	$(CC) $(CFLAGS) -o rdt_drbg $(DRBG_OBJ)

rdt_core.o: src/rdt_core.c $(HDR)
	$(CC) $(CFLAGS) -c src/rdt_core.c

rdt_prng.o: src/rdt_prng.c $(HDR)
	$(CC) $(CFLAGS) -c src/rdt_prng.c

rdt_drbg.o: src/rdt_drbg.c $(HDR)
	$(CC) $(CFLAGS) -c src/rdt_drbg.c

clean:
	rm -f *.o rdt_prng rdt_drbg

.PHONY: all clean
