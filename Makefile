CC = gcc
CFLAGS = -O3 -std=c11 -march=native -Wall -Wextra -Wshadow -Wconversion
HDR = src/rdt_core.h src/rdt.h
HDR_V2 = src/rdt256_stream_v2.h
HDR_SEED = src/rdt_seed_extractor.h
CORE_OBJ = rdt_core.o
PRNG_OBJ = rdt_core.o rdt_prng.o
STREAM_OBJ = rdt_core.o rdt_prng_stream.o
STREAM_V2_MAIN_OBJ = rdt256_stream_v2_main.o
STREAM_V3_OBJ = rdt256_stream_v3.o
DRBG_OBJ = rdt_core.o rdt_drbg.o
SEED_OBJ = rdt_seed_extractor.o

all: rdt_prng_stream rdt_prng_stream_v2 rdt_prng_stream_v3 rdt_seed_extractor

# ---------- binaries ----------
rdt_prng: $(PRNG_OBJ)
	$(CC) $(CFLAGS) -o $@ $(PRNG_OBJ)

rdt_prng_stream: $(STREAM_OBJ)
	$(CC) $(CFLAGS) -o $@ $(STREAM_OBJ)

rdt_prng_stream_v2: $(STREAM_V2_MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $(STREAM_V2_MAIN_OBJ)

rdt_prng_stream_v3: $(STREAM_V3_OBJ)
	$(CC) $(CFLAGS) -o $@ $(STREAM_V3_OBJ)

rdt_drbg: $(DRBG_OBJ)
	$(CC) $(CFLAGS) -o $@ $(DRBG_OBJ)

rdt_seed_extractor: $(SEED_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SEED_OBJ)

# ---------- objects ----------
rdt_core.o: src/rdt_core.c $(HDR)
	$(CC) $(CFLAGS) -c $<

rdt_prng.o: src/rdt_prng.c $(HDR)
	$(CC) $(CFLAGS) -c $<

rdt_prng_stream.o: src/rdt_prng_stream.c $(HDR)
	$(CC) $(CFLAGS) -c $<

rdt256_stream_v2_main.o: src/rdt256_stream_v2.c $(HDR_V2)
	$(CC) $(CFLAGS) -DRDT_PRNG_V2_MAIN -c $< -o $@

rdt256_stream_v3.o: src/rdt256_stream_v2.c $(HDR_V2)
	$(CC) $(CFLAGS) -DRDT_PRNG_V3_MAIN -c $< -o $@

rdt_drbg.o: src/rdt_drbg.c $(HDR)
	$(CC) $(CFLAGS) -c $<

rdt_seed_extractor.o: src/rdt_seed_extractor.c $(HDR_SEED)
	$(CC) $(CFLAGS) -DRDT_SEED_EXTRACTOR_MAIN -c $<

# ---------- test targets ----------
test-v2-dieharder: rdt_prng_stream_v2
	./rdt_prng_stream_v2 | dieharder -a -g 200

test-v2-smokerand: rdt_prng_stream_v2
	./rdt_prng_stream_v2 | smokerand default stdin64

test-v2-ent: rdt_prng_stream_v2
	./rdt_prng_stream_v2 | head -c 10000000 | ent

benchmark-v2: rdt_prng_stream_v2
	@echo "Benchmarking v2: 256 MiB output..."
	@time ./rdt_prng_stream_v2 | head -c 268435456 > /dev/null

splitmix64_stream: benchmarks/splitmix64_stream.c
	$(CC) $(CFLAGS) -o $@ $<

benchmark-honest: rdt_prng_stream_v2 rdt_prng_stream_v3 splitmix64_stream
	python3 benchmarks/benchmark_streams.py --sample-mib 64

# ---------- debug build ----------
debug: CFLAGS += -g -O0 -fsanitize=address -fsanitize=undefined -DDEBUG
debug: clean all

# ---------- housekeeping ----------
clean:
	rm -f *.o rdt_prng_stream rdt_prng_stream_v2 rdt_prng_stream_v3 rdt_seed_extractor splitmix64_stream

.PHONY: all clean debug test-v2-dieharder test-v2-smokerand test-v2-ent benchmark-v2 benchmark-honest
