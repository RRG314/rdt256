# RDT256 Honest Stream Benchmark

- sample per generator: `64 MiB`

| generator | throughput MiB/s | entropy | monobit | lag1 corr(u64) | quality_proxy (lower better) |
|---|---:|---:|---:|---:|---:|
| rdt_prng_stream_v2 | 48.34 | 8.00000 | 0.50001 | -0.000239 | 0.000245 |
| rdt_prng_stream_v3 | 47.72 | 8.00000 | 0.50002 | 0.000189 | 0.000205 |
| splitmix64 | 1329.18 | 8.00000 | 0.49999 | -0.000146 | 0.000159 |

## Findings
- v3 speedup vs v2: `0.987x`
- v2 speed ratio vs SplitMix64: `0.036`
- v3 speed ratio vs SplitMix64: `0.036`
- Entropy delta (v3-v2): `0.000000` bits/byte
- |lag1| delta (v3-v2): `-0.000050`
- quality_proxy delta (v3-v2): `-0.000040` (negative means v3 better)
- This is a statistical/throughput comparison only; not a security proof.
