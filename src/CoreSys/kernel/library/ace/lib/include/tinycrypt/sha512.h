#ifndef __TC_SHA512_H__
#define __TC_SHA512_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TC_SHA512_BLOCK_SIZE  128
#define TC_SHA512_DIGEST_SIZE 64
#define TC_SHA512_STATE_BLOCKS 8

struct tc_sha512_state_struct {
    uint64_t iv[TC_SHA512_STATE_BLOCKS];

    /*
     * 128-bit message length.
     *
     * bits_hashed_hi = upper 64 bits
     * bits_hashed_lo = lower 64 bits
     */
    uint64_t bits_hashed_hi;
    uint64_t bits_hashed_lo;

    uint8_t leftover[TC_SHA512_BLOCK_SIZE];
    size_t leftover_offset;
};

typedef struct tc_sha512_state_struct *TCSha512State_t;

int tc_sha512_init(TCSha512State_t s);

int tc_sha512_update(
    TCSha512State_t s,
    const uint8_t *data,
    size_t datalen
);

int tc_sha512_final(
    uint8_t *digest,
    TCSha512State_t s
);

int tc_sha384_init(TCSha512State_t s);

int tc_sha384_update(
    TCSha512State_t s,
    const uint8_t *data,
    size_t datalen
);

int tc_sha384_final(
    uint8_t *digest,
    TCSha512State_t s
);


#ifdef __cplusplus
}
#endif

#endif
