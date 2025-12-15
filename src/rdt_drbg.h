#ifndef RDT_DRBG_H
#define RDT_DRBG_H

#include <stdint.h>

/* DRBG interface */
void     rdt_drbg_init(uint64_t seed);
uint64_t rdt_drbg_next(void);

#endif /* RDT_DRBG_H */
