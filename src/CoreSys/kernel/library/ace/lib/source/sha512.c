/*
 * sha512.c
 *
 * SHA-512 / SHA-384 implementation for TinyCrypt / CoreSys ACE.
 *
 * SHA-512:
 *   Block size : 128 bytes
 *   Digest     : 64 bytes
 *   Word size  : 64 bits
 *   Rounds     : 80
 *
 * SHA-384:
 *   Block size : 128 bytes
 *   Digest     : 48 bytes
 *   Word size  : 64 bits
 *   Rounds     : 80
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tinycrypt/sha512.h>
#include <tinycrypt/constants.h>
#include <tinycrypt/utils.h>

static void compress(uint64_t *iv, const uint8_t *data);

/* ============================================================
 * SHA-512 constants
 * ============================================================ */

static const uint64_t k512[80] = {
    0x428a2f98d728ae22ULL,
    0x7137449123ef65cdULL,
    0xb5c0fbcfec4d3b2fULL,
    0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL,
    0x59f111f1b605d019ULL,
    0x923f82a4af194f9bULL,
    0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL,
    0x12835b0145706fbeULL,
    0x243185be4ee4b28cULL,
    0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL,
    0x80deb1fe3b1696b1ULL,
    0x9bdc06a725c71235ULL,
    0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL,
    0xefbe4786384f25e3ULL,
    0x0fc19dc68b8cd5b5ULL,
    0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL,
    0x4a7484aa6ea6e483ULL,
    0x5cb0a9dcbd41fbd4ULL,
    0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL,
    0xa831c66d2db43210ULL,
    0xb00327c898fb213fULL,
    0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL,
    0xd5a79147930aa725ULL,
    0x06ca6351e003826fULL,
    0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL,
    0x2e1b21385c26c926ULL,
    0x4d2c6dfc5ac42aedULL,
    0x53380d139d95b3dfULL,
    0x650a73548baf63deULL,
    0x766a0abb3c77b2a8ULL,
    0x81c2c92e47edaee6ULL,
    0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL,
    0xa81a664bbc423001ULL,
    0xc24b8b70d0f89791ULL,
    0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL,
    0xd69906245565a910ULL,
    0xf40e35855771202aULL,
    0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL,
    0x1e376c085141ab53ULL,
    0x2748774cdf8eeb99ULL,
    0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL,
    0x4ed8aa4ae3418acbULL,
    0x5b9cca4f7763e373ULL,
    0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL,
    0x78a5636f43172f60ULL,
    0x84c87814a1f0ab72ULL,
    0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL,
    0xa4506cebde82bde9ULL,
    0xbef9a3f7b2c67915ULL,
    0xc67178f2e372532bULL,
    0xca273eceea26619cULL,
    0xd186b8c721c0c207ULL,
    0xeada7dd6cde0eb1eULL,
    0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL,
    0x0a637dc5a2c898a6ULL,
    0x113f9804bef90daeULL,
    0x1b710b35131c471bULL,
    0x28db77f523047d84ULL,
    0x32caab7b40c72493ULL,
    0x3c9ebe0a15c9bebcULL,
    0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL,
    0x597f299cfc657e2aULL,
    0x5fcb6fab3ad6faecULL,
    0x6c44198c4a475817ULL
};

/* ============================================================
 * Initial vectors
 * ============================================================ */

static const uint64_t sha512_iv[8] = {
    0x6a09e667f3bcc908ULL,
    0xbb67ae8584caa73bULL,
    0x3c6ef372fe94f82bULL,
    0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL,
    0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL,
    0x5be0cd19137e2179ULL
};

static const uint64_t sha384_iv[8] = {
    0xcbbb9d5dc1059ed8ULL,
    0x629a292a367cd507ULL,
    0x9159015a3070dd17ULL,
    0x152fecd8f70e5939ULL,
    0x67332667ffc00b31ULL,
    0x8eb44a8768581511ULL,
    0xdb0c2e0d64f98fa7ULL,
    0x47b5481dbefa4fa4ULL
};

/* ============================================================
 * SHA-512 functions
 * ============================================================ */

static inline uint64_t rotr64(uint64_t x, unsigned int n)
{
    return (x >> n) | (x << (64U - n));
}

#define SIGMA0(x) \
    (rotr64((x), 28) ^ rotr64((x), 34) ^ rotr64((x), 39))

#define SIGMA1(x) \
    (rotr64((x), 14) ^ rotr64((x), 18) ^ rotr64((x), 41))

#define sigma0(x) \
    (rotr64((x), 1) ^ rotr64((x), 8) ^ ((x) >> 7))

#define sigma1(x) \
    (rotr64((x), 19) ^ rotr64((x), 61) ^ ((x) >> 6))

#define Ch(x, y, z) \
    (((x) & (y)) ^ (~(x) & (z)))

#define Maj(x, y, z) \
    (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

/* ============================================================
 * Big-endian 64-bit load
 * ============================================================ */

static inline uint64_t BigEndian64(const uint8_t **p)
{
    uint64_t n;

    n  = ((uint64_t)(*((*p)++))) << 56;
    n |= ((uint64_t)(*((*p)++))) << 48;
    n |= ((uint64_t)(*((*p)++))) << 40;
    n |= ((uint64_t)(*((*p)++))) << 32;
    n |= ((uint64_t)(*((*p)++))) << 24;
    n |= ((uint64_t)(*((*p)++))) << 16;
    n |= ((uint64_t)(*((*p)++))) << 8;
    n |= ((uint64_t)(*((*p)++)));

    return n;
}

/* ============================================================
 * Big-endian 64-bit store
 * ============================================================ */

static inline void Store64BE(uint8_t *p, uint64_t x)
{
    p[0] = (uint8_t)(x >> 56);
    p[1] = (uint8_t)(x >> 48);
    p[2] = (uint8_t)(x >> 40);
    p[3] = (uint8_t)(x >> 32);
    p[4] = (uint8_t)(x >> 24);
    p[5] = (uint8_t)(x >> 16);
    p[6] = (uint8_t)(x >> 8);
    p[7] = (uint8_t)x;
}

/* ============================================================
 * Compression function
 * ============================================================ */

static void compress(uint64_t *iv, const uint8_t *data)
{
    uint64_t a;
    uint64_t b;
    uint64_t c;
    uint64_t d;
    uint64_t e;
    uint64_t f;
    uint64_t g;
    uint64_t h;

    uint64_t t1;
    uint64_t t2;

    uint64_t s0;
    uint64_t s1;

    uint64_t w[16];

    unsigned int i;

    a = iv[0];
    b = iv[1];
    c = iv[2];
    d = iv[3];
    e = iv[4];
    f = iv[5];
    g = iv[6];
    h = iv[7];

    /*
     * First 16 rounds.
     */
    for (i = 0; i < 16; ++i) {
        w[i] = BigEndian64(&data);

        t1 =
            h +
            SIGMA1(e) +
            Ch(e, f, g) +
            k512[i] +
            w[i];

        t2 =
            SIGMA0(a) +
            Maj(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    /*
     * Remaining 64 rounds.
     *
     * A 16-word rolling message schedule is used instead of
     * storing all 80 words.
     */
    for (i = 16; i < 80; ++i) {
        unsigned int index = i & 15;

        s0 = sigma0(w[(i + 1) & 15]);
        s1 = sigma1(w[(i + 14) & 15]);

        w[index] =
            w[index] +
            s0 +
            w[(i + 9) & 15] +
            s1;

        t1 =
            h +
            SIGMA1(e) +
            Ch(e, f, g) +
            k512[i] +
            w[index];

        t2 =
            SIGMA0(a) +
            Maj(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    iv[0] += a;
    iv[1] += b;
    iv[2] += c;
    iv[3] += d;
    iv[4] += e;
    iv[5] += f;
    iv[6] += g;
    iv[7] += h;
}

/* ============================================================
 * SHA-512 initialization
 * ============================================================ */

int tc_sha512_init(TCSha512State_t s)
{
    unsigned int i;

    if (s == (TCSha512State_t)0)
        return TC_CRYPTO_FAIL;

    _set((uint8_t *)s, 0x00, sizeof(*s));

    for (i = 0; i < 8; ++i)
        s->iv[i] = sha512_iv[i];

    return TC_CRYPTO_SUCCESS;
}

/* ============================================================
 * SHA-512 update
 * ============================================================ */

int tc_sha512_update(
    TCSha512State_t s,
    const uint8_t *data,
    size_t datalen)
{
    if (s == (TCSha512State_t)0)
        return TC_CRYPTO_FAIL;

    if (data == (const uint8_t *)0)
        return TC_CRYPTO_FAIL;

    if (datalen == 0)
        return TC_CRYPTO_SUCCESS;

    while (datalen > 0) {
        size_t available =
            TC_SHA512_BLOCK_SIZE - s->leftover_offset;

        size_t take =
            datalen < available ? datalen : available;

        unsigned int i;

        for (i = 0; i < take; ++i) {
            s->leftover[s->leftover_offset + i] = data[i];
        }

        s->leftover_offset += take;
        data += take;
        datalen -= take;

        if (s->leftover_offset == TC_SHA512_BLOCK_SIZE) {

            compress(s->iv, s->leftover);

            /*
             * 128 bytes = 1024 bits.
             *
             * Add 1024 to the 128-bit message length.
             */
            s->bits_hashed_lo += 1024ULL;

            if (s->bits_hashed_lo < 1024ULL)
                s->bits_hashed_hi++;

            s->leftover_offset = 0;
        }
    }

    return TC_CRYPTO_SUCCESS;
}

/* ============================================================
 * SHA-512 final
 * ============================================================ */

int tc_sha512_final(
    uint8_t *digest,
    TCSha512State_t s)
{
    unsigned int i;

    if (digest == (uint8_t *)0)
        return TC_CRYPTO_FAIL;

    if (s == (TCSha512State_t)0)
        return TC_CRYPTO_FAIL;

    /*
     * Add the number of bits currently sitting in leftover.
     */
    {
        uint64_t remaining_bits =
            ((uint64_t)s->leftover_offset) << 3;

        uint64_t old_lo = s->bits_hashed_lo;

        s->bits_hashed_lo += remaining_bits;

        if (s->bits_hashed_lo < old_lo)
            s->bits_hashed_hi++;
    }

    /*
     * Append 0x80.
     */
    s->leftover[s->leftover_offset++] = 0x80;

    /*
     * SHA-512 has a 128-bit length field.
     *
     * We need 16 bytes at the end of the final block.
     */
    if (s->leftover_offset > (TC_SHA512_BLOCK_SIZE - 16)) {

        /*
         * Zero remaining bytes in this block.
         */
        _set(
            s->leftover + s->leftover_offset,
            0x00,
            TC_SHA512_BLOCK_SIZE - s->leftover_offset
        );

        compress(s->iv, s->leftover);

        s->leftover_offset = 0;
    }

    /*
     * Zero everything before the 128-bit length.
     */
    _set(
        s->leftover + s->leftover_offset,
        0x00,
        TC_SHA512_BLOCK_SIZE - 16 - s->leftover_offset
    );

    /*
     * Write high 64 bits of message length.
     */
    Store64BE(
        s->leftover + TC_SHA512_BLOCK_SIZE - 16,
        s->bits_hashed_hi
    );

    /*
     * Write low 64 bits of message length.
     */
    Store64BE(
        s->leftover + TC_SHA512_BLOCK_SIZE - 8,
        s->bits_hashed_lo
    );

    /*
     * Compress final block.
     */
    compress(s->iv, s->leftover);

    /*
     * Output 512-bit digest.
     */
    for (i = 0; i < 8; ++i)
        Store64BE(digest + (i * 8), s->iv[i]);

    /*
     * Clear sensitive state.
     */
    _set((uint8_t *)s, 0x00, sizeof(*s));

    return TC_CRYPTO_SUCCESS;
}

/* ============================================================
 * SHA-384 initialization
 *
 * SHA-384 uses exactly the same compression function as SHA-512.
 * Only the IV and digest size differ.
 * ============================================================ */

int tc_sha384_init(TCSha512State_t s)
{
    unsigned int i;

    if (s == (TCSha512State_t)0)
        return TC_CRYPTO_FAIL;

    _set((uint8_t *)s, 0x00, sizeof(*s));

    for (i = 0; i < 8; ++i)
        s->iv[i] = sha384_iv[i];

    return TC_CRYPTO_SUCCESS;
}

/* ============================================================
 * SHA-384 final
 * ============================================================ */

int tc_sha384_final(
    uint8_t *digest,
    TCSha512State_t s)
{
    unsigned int i;

    if (digest == (uint8_t *)0)
        return TC_CRYPTO_FAIL;

    if (s == (TCSha512State_t)0)
        return TC_CRYPTO_FAIL;

    /*
     * Add leftover bits to the 128-bit message length.
     */
    {
        uint64_t remaining_bits =
            ((uint64_t)s->leftover_offset) << 3;

        uint64_t old_lo = s->bits_hashed_lo;

        s->bits_hashed_lo += remaining_bits;

        if (s->bits_hashed_lo < old_lo)
            s->bits_hashed_hi++;
    }

    /*
     * Append 0x80.
     */
    s->leftover[s->leftover_offset++] = 0x80;

    /*
     * SHA-384 has the same 128-bit length field as SHA-512.
     */
    if (s->leftover_offset > (TC_SHA512_BLOCK_SIZE - 16)) {

        _set(
            s->leftover + s->leftover_offset,
            0x00,
            TC_SHA512_BLOCK_SIZE - s->leftover_offset
        );

        compress(s->iv, s->leftover);

        s->leftover_offset = 0;
    }

    /*
     * Zero padding before the length.
     */
    _set(
        s->leftover + s->leftover_offset,
        0x00,
        TC_SHA512_BLOCK_SIZE - 16 - s->leftover_offset
    );

    /*
     * 128-bit length.
     */
    Store64BE(
        s->leftover + TC_SHA512_BLOCK_SIZE - 16,
        s->bits_hashed_hi
    );

    Store64BE(
        s->leftover + TC_SHA512_BLOCK_SIZE - 8,
        s->bits_hashed_lo
    );

    /*
     * Final compression.
     */
    compress(s->iv, s->leftover);

    /*
     * SHA-384 outputs only the first six 64-bit words.
     *
     * 6 * 8 = 48 bytes.
     */
    for (i = 0; i < 6; ++i)
        Store64BE(digest + (i * 8), s->iv[i]);

    /*
     * Destroy state.
     */
    _set((uint8_t *)s, 0x00, sizeof(*s));

    return TC_CRYPTO_SUCCESS;
}
