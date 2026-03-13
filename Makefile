CC = gcc
CFLAGS = -O3 -std=c11 -march=native -Wall -Wextra -Wshadow -Wconversion
HDR = src/rdt_core.h src/rdt.h
HDR_V2 = src/rdt256_stream_v2.h
HDR_SEED = src/rdt_seed_extractor.h
HDR_HASH = src/rdt_sha256.h
HDR_DRBG_V2 = src/rdt_drbg_v2.h $(HDR_HASH)
CORE_OBJ = rdt_core.o
PRNG_OBJ = rdt_core.o rdt_prng.o
STREAM_OBJ = rdt_core.o rdt_prng_stream.o
STREAM_V2_MAIN_OBJ = rdt256_stream_v2_main.o
STREAM_V3_OBJ = rdt256_stream_v3.o
HASH_OBJ = rdt_sha256.o
DRBG_OBJ = rdt_core.o rdt_drbg.o rdt_drbg_stream.o
DRBG_V2_OBJ = rdt_core.o rdt_sha256.o rdt_drbg_v2.o rdt_drbg_v2_stream.o
SEED_OBJ = rdt_seed_extractor.o

all: rdt_prng_stream rdt_prng_stream_v2 rdt_prng_stream_v3 rdt_drbg rdt_drbg_v2 rdt_seed_extractor

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

rdt_drbg_v2: $(DRBG_V2_OBJ)
	$(CC) $(CFLAGS) -o $@ $(DRBG_V2_OBJ)

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

rdt_drbg_stream.o: src/rdt_drbg_stream.c src/rdt_drbg.h
	$(CC) $(CFLAGS) -c $<

rdt_sha256.o: src/rdt_sha256.c $(HDR_HASH)
	$(CC) $(CFLAGS) -c $<

rdt_drbg_v2.o: src/rdt_drbg_v2.c src/rdt_core.h $(HDR_DRBG_V2)
	$(CC) $(CFLAGS) -c $<

rdt_drbg_v2_stream.o: src/rdt_drbg_v2_stream.c $(HDR_DRBG_V2)
	$(CC) $(CFLAGS) -c $<

rdt_seed_extractor.o: src/rdt_seed_extractor.c $(HDR_SEED)
	$(CC) $(CFLAGS) -DRDT_SEED_EXTRACTOR_MAIN -c $<

# ---------- test targets ----------
test-v2-dieharder: rdt_prng_stream_v2
	./rdt_prng_stream_v2 | dieharder -a -g 200

test-drbg-v2: rdt_drbg_v2
	./rdt_drbg_v2 | head -c 10000000 > /dev/null

test-drbg-v2-kat: rdt_sha256.o rdt_core.o rdt_drbg_v2.o tests/rdt_drbg_v2_test.c
	$(CC) $(CFLAGS) -I./src tests/rdt_drbg_v2_test.c rdt_core.o rdt_sha256.o rdt_drbg_v2.o -o rdt_drbg_v2_test
	./rdt_drbg_v2_test

test-drbg-v2-system: rdt_sha256.o rdt_core.o rdt_drbg_v2.o tests/rdt_drbg_v2_system_test.c
	$(CC) $(CFLAGS) -I./src tests/rdt_drbg_v2_system_test.c rdt_core.o rdt_sha256.o rdt_drbg_v2.o -o rdt_drbg_v2_system_test
	./rdt_drbg_v2_system_test

test-seed-extractor: tests/rdt_seed_extractor_test.c src/rdt_seed_extractor.c $(HDR_SEED)
	$(CC) $(CFLAGS) -I./src tests/rdt_seed_extractor_test.c src/rdt_seed_extractor.c -o rdt_seed_extractor_test
	./rdt_seed_extractor_test

validate-seed-extractor: rdt_seed_extractor
	python3 tests/validate_seed_extractor.py

test-v2-smokerand: rdt_prng_stream_v2
	./rdt_prng_stream_v2 | smokerand default stdin64

test-v2-ent: rdt_prng_stream_v2
	./rdt_prng_stream_v2 | head -c 10000000 | ent

benchmark-v2: rdt_prng_stream_v2
	@echo "Benchmarking v2: 256 MiB output..."
	@time ./rdt_prng_stream_v2 | head -c 268435456 > /dev/null

splitmix64_stream: benchmarks/splitmix64_stream.c
	$(CC) $(CFLAGS) -o $@ $<

benchmark-honest: rdt_prng_stream_v2 rdt_prng_stream_v3 rdt_drbg_v2 splitmix64_stream
	python3 benchmarks/benchmark_streams.py --sample-mib 64

test-all: test-drbg-v2-kat test-drbg-v2-system test-seed-extractor
	python3 tests/run_results.py
	python3 tests/validate_seed_extractor.py

# ---------- debug build ----------
debug: CFLAGS += -g -O0 -fsanitize=address -fsanitize=undefined -DDEBUG
debug: clean all

# ---------- housekeeping ----------
clean:
	rm -f *.o rdt_prng_stream rdt_prng_stream_v2 rdt_prng_stream_v3 rdt_drbg rdt_drbg_v2 rdt_seed_extractor splitmix64_stream rdt_drbg_v2_test rdt_drbg_v2_system_test rdt_seed_extractor_test

.PHONY: all clean debug test-v2-dieharder test-v2-smokerand test-v2-ent test-drbg-v2 test-drbg-v2-kat test-drbg-v2-system test-seed-extractor validate-seed-extractor test-all benchmark-v2 benchmark-honest
