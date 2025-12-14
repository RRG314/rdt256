CC = gcc
CFLAGS = -O3 -std=c11 -march=native -Wall -Wextra -Wshadow -Wconversion

HDR = src/rdt_core.h src/rdt.h

CORE_OBJ = rdt_core.o
PRNG_OBJ = rdt_core.o rdt_prng.o
STREAM_OBJ = rdt_core.o rdt_prng_stream.o
DRBG_OBJ = rdt_core.o rdt_drbg.o

all: rdt_prng rdt_prng_stream rdt_drbg

# ---------- binaries ----------

rdt_prng: $(PRNG_OBJ)
	$(CC) $(CFLAGS) -o $@ $(PRNG_OBJ)

rdt_prng_stream: $(STREAM_OBJ)
	$(CC) $(CFLAGS) -o $@ $(STREAM_OBJ)

rdt_drbg: $(DRBG_OBJ)
	$(CC) $(CFLAGS) -o $@ $(DRBG_OBJ)

# ---------- objects ----------

rdt_core.o: src/rdt_core.c $(HDR)
	$(CC) $(CFLAGS) -c $<

rdt_prng.o: src/rdt_prng.c $(HDR)
	$(CC) $(CFLAGS) -c $<

rdt_prng_stream.o: src/rdt_prng_stream.c $(HDR)
	$(CC) $(CFLAGS) -c $<

rdt_drbg.o: src/rdt_drbg.c $(HDR)
	$(CC) $(CFLAGS) -c $<

# ---------- housekeeping ----------

clean:
	rm -f *.o rdt_prng rdt_prng_stream rdt_drbg

.PHONY: all clean
