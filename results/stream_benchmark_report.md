# RDT256 Honest Stream Benchmark

- sample per generator: `64 MiB`

| generator | throughput MiB/s | entropy | monobit | lag1 corr(u64) | quality_proxy (lower better) |
|---|---:|---:|---:|---:|---:|
| rdt_prng_stream_v2 | 55.77 | 8.00000 | 0.50001 | -0.000239 | 0.000245 |
| rdt_prng_stream_v3 | 47.44 | 8.00000 | 0.50002 | 0.000189 | 0.000205 |
| rdt_drbg_v2 | 23.59 | 8.00000 | 0.50000 | 0.000117 | 0.000122 |
| splitmix64 | 209.78 | 8.00000 | 0.49999 | -0.000146 | 0.000159 |
| openssl_rand | 353.92 | 8.00000 | 0.50004 | -0.000435 | 0.000476 |

## Findings
- v3 speedup vs v2: `0.851x`
- v2 speed ratio vs SplitMix64: `0.266`
- v3 speed ratio vs SplitMix64: `0.226`
- DRBG v2 speed ratio vs SplitMix64: `0.112`
- Entropy delta (v3-v2): `0.000000` bits/byte
- |lag1| delta (v3-v2): `-0.000050`
- quality_proxy delta (v3-v2): `-0.000040` (negative means v3 better)
- DRBG v2 quality_proxy: `0.000122`
- DRBG v2 speed ratio vs OpenSSL rand: `0.067`
- DRBG v2 quality_proxy delta vs OpenSSL rand: `-0.000355`
- This is a statistical/throughput comparison only; not a security proof.
