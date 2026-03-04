# RDT256 Honest Stream Benchmark

- sample per generator: `64 MiB`

| generator | throughput MiB/s | entropy | monobit | lag1 corr(u64) | quality_proxy (lower better) |
|---|---:|---:|---:|---:|---:|
| rdt_prng_stream_v2 | 61.14 | 8.00000 | 0.50001 | -0.000239 | 0.000245 |
| rdt_prng_stream_v3 | 50.00 | 8.00000 | 0.50002 | 0.000189 | 0.000205 |
| splitmix64 | 949.01 | 8.00000 | 0.49999 | -0.000146 | 0.000159 |

## Findings
- v3 speedup vs v2: `0.818x`
- v2 speed ratio vs SplitMix64: `0.064`
- v3 speed ratio vs SplitMix64: `0.053`
- Entropy delta (v3-v2): `0.000000` bits/byte
- |lag1| delta (v3-v2): `-0.000050`
- quality_proxy delta (v3-v2): `-0.000040` (negative means v3 better)
- This is a statistical/throughput comparison only; not a security proof.
