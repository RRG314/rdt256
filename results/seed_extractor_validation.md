# Seed Extractor Validation

- fixture: `examples/sensor_data.csv`
- fixture bytes: `600`
- single-file seed: `f2cb746be11d12125cacd7d2b3e09a9dd7f9775dde8e12edfdc0391a920b1e8e`
- double-file seed: `654a9d34e91286afe2203ee6eddfd108021bd966046f737bea5b6f5b94af60a1`
- deterministic repeated runs: `8`
- average CLI time: `0.002210 s`
- median CLI time: `0.002079 s`
- throughput: `0.259 MiB/s`
- avalanche mean: `128.00 / 256 bits`
- avalanche range: `117` to `142` bits
- unique mutated outputs: `6` / `6`

## Avalanche Samples

| byte position | flipped bits in output seed |
|---|---:|
| 0 | 120 |
| 120 | 127 |
| 240 | 117 |
| 360 | 129 |
| 480 | 142 |
| 599 | 133 |
