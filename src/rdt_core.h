#ifndef RDT_CORE_H
#define RDT_CORE_H

#include <stdint.h>

/* Core nonlinear RDT mixing primitive */
uint64_t rdt_mix(uint64_t x, const uint64_t K[4]);

#endif /* RDT_CORE_H */
