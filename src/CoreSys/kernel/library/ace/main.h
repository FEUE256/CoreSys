// Advanced Crypto Engine

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <ame/main.h>

#include <tinycrypt/aes.h>
#include <tinycrypt/sha256.h>
#include <tinycrypt/sha512.h>
#include <tinycrypt/hmac.h>
#include <tinycrypt/cmac_mode.h>
#include <tinycrypt/ctr_mode.h>
#include <tinycrypt/cbc_mode.h>
#include <tinycrypt/ccm_mode.h>
#include <tinycrypt/hmac_prng.h>
#include <tinycrypt/ctr_prng.h>
#include <tinycrypt/ecc.h>
#include <tinycrypt/ecc_dh.h>
#include <tinycrypt/ecc_dsa.h>
#include <tinycrypt/utils.h>

#include <ace/rsa/rsa.h>
#include <ace/rsa/bigint.h>

/* ace/main.h
 *
 * Crypto To C
 * ACE - Advanced Crypto Engine
 * CoreSys Copyright 2026 FÈUE License
 */

/*
    Implements:
        AES-128
        ECC
        ECDH
        ECDSA
        SHA-256
        SHA-512
        SHA-384
        HMAC-SHA256
        HMAC-PRNG
        CTR-PRNG
        CMAC
        CTR
        CBC
        CCM
        RSA
*/

/* ============================================================
   AES-128
   ============================================================ */

int ace_aes_encrypt(uint8_t *out,
                    const uint8_t *in,
                    const uint8_t *key);

int ace_aes_decrypt(uint8_t *out,
                    const uint8_t *in,
                    const uint8_t *key);


/* ============================================================
   SHA-256
   ============================================================ */

int ace_sha256(uint8_t *digest,
               const uint8_t *data,
               size_t len);

int ace_sha512(uint8_t *digest,
               const uint8_t *data,
               size_t len);

int ace_sha384(uint8_t *digest,
               const uint8_t *data,
               size_t len);

/* ============================================================
   HMAC-SHA256
   ============================================================ */

int ace_hmac_sha256(uint8_t *mac,
                    const uint8_t *key,
                    size_t key_len,
                    const uint8_t *data,
                    size_t data_len);


/* ============================================================
   CMAC
   ============================================================ */

int ace_cmac(uint8_t *mac,
             const uint8_t *key,
             const uint8_t *data,
             size_t data_len);


/* ============================================================
   CTR
   ============================================================ */

int ace_ctr(uint8_t *out,
            const uint8_t *in,
            size_t len,
            const uint8_t *key,
            uint8_t *ctr);


/* ============================================================
   CBC

   TinyCrypt CBC encryption prepends the IV to ciphertext.
   Therefore:
       encrypt: outlen = inlen + 16
       decrypt: input contains IV + ciphertext
   ============================================================ */

int ace_cbc_encrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *in,
                    size_t in_len,
                    const uint8_t *iv,
                    const uint8_t *key);

int ace_cbc_decrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *in,
                    size_t in_len,
                    const uint8_t *key);


/* ============================================================
   CCM
   ============================================================ */

int ace_ccm_encrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *aad,
                    size_t aad_len,
                    const uint8_t *plaintext,
                    size_t plaintext_len,
                    uint8_t *nonce,
                    size_t nonce_len,
                    unsigned int tag_len,
                    const uint8_t *key);

int ace_ccm_decrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *aad,
                    size_t aad_len,
                    const uint8_t *ciphertext,
                    size_t ciphertext_len,
                    uint8_t *nonce,
                    size_t nonce_len,
                    unsigned int tag_len,
                    const uint8_t *key);


/* ============================================================
   HMAC-PRNG
   ============================================================ */

int ace_hmac_prng_init(TCHmacPrng_t ctx,
                       const uint8_t *personalization,
                       size_t personalization_len);

int ace_hmac_prng_reseed(TCHmacPrng_t ctx,
                         const uint8_t *seed,
                         size_t seed_len,
                         const uint8_t *additional_input,
                         size_t additional_input_len);

int ace_hmac_prng_generate(TCHmacPrng_t ctx,
                           uint8_t *out,
                           size_t len);


/* ============================================================
   CTR-PRNG
   ============================================================ */

int ace_ctr_prng_init(TCCtrPrng_t *ctx,
                      const uint8_t *entropy,
                      size_t entropy_len,
                      const uint8_t *personalization,
                      size_t personalization_len);

int ace_ctr_prng_reseed(TCCtrPrng_t *ctx,
                        const uint8_t *entropy,
                        size_t entropy_len,
                        const uint8_t *additional_input,
                        size_t additional_input_len);

int ace_ctr_prng_generate(TCCtrPrng_t *ctx,
                          const uint8_t *additional_input,
                          size_t additional_input_len,
                          uint8_t *out,
                          size_t len);

void ace_ctr_prng_uninstantiate(TCCtrPrng_t *ctx);


/* ============================================================
   ECC
   ============================================================ */

void ace_ecc_set_rng(uECC_RNG_Function rng);

uECC_RNG_Function ace_ecc_get_rng(void);

uECC_Curve ace_ecc_secp256r1(void);

int ace_ecc_private_key_size(uECC_Curve curve);

int ace_ecc_public_key_size(uECC_Curve curve);

int ace_ecc_make_key(uint8_t *public_key,
                     uint8_t *private_key,
                     uECC_Curve curve);

int ace_ecc_compute_public_key(const uint8_t *private_key,
                               uint8_t *public_key,
                               uECC_Curve curve);

int ace_ecc_validate_point(const uECC_word_t *point,
                           uECC_Curve curve);

int ace_ecc_validate_public_key(const uint8_t *public_key,
                                uECC_Curve curve);


/* ============================================================
   ECDH
   ============================================================ */

int ace_ecdh_shared_secret(const uint8_t *public_key,
                           const uint8_t *private_key,
                           uint8_t *secret,
                           uECC_Curve curve);


/* ============================================================
   ECDSA
   ============================================================ */

int ace_ecdsa_sign(const uint8_t *private_key,
                   const uint8_t *hash,
                   unsigned int hash_len,
                   uint8_t *signature,
                   uECC_Curve curve);

int ace_ecdsa_verify(const uint8_t *public_key,
                     const uint8_t *hash,
                     unsigned int hash_len,
                     const uint8_t *signature,
                     uECC_Curve curve);


/* ============================================================
   Constant-time utility
   ============================================================ */

int ace_crypto_compare(const uint8_t *a,
                       const uint8_t *b,
                       size_t len);

/* ============================================================
   AES-128
   ============================================================ */

int ace_aes_encrypt(uint8_t *out,
                    const uint8_t *in,
                    const uint8_t *key)
{
    struct tc_aes_key_sched_struct sched;

    if (out == NULL || in == NULL || key == NULL)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_encrypt_key(&sched, key) != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    return tc_aes_encrypt(out, in, &sched);
}


int ace_aes_decrypt(uint8_t *out,
                    const uint8_t *in,
                    const uint8_t *key)
{
    struct tc_aes_key_sched_struct sched;

    if (out == NULL || in == NULL || key == NULL)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_decrypt_key(&sched, key) != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    return tc_aes_decrypt(out, in, &sched);
}


/* ============================================================
   SHA-256
   ============================================================ */

int ace_sha256(uint8_t *digest,
               const uint8_t *data,
               size_t len)
{
    struct tc_sha256_state_struct state;

    if (digest == NULL)
        return TC_CRYPTO_FAIL;

    if (data == NULL && len != 0)
        return TC_CRYPTO_FAIL;

    if (tc_sha256_init(&state) != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (len != 0) {
        if (tc_sha256_update(&state, data, len) != TC_CRYPTO_SUCCESS)
            return TC_CRYPTO_FAIL;
    }

    return tc_sha256_final(digest, &state);
}


/* ============================================================
   HMAC-SHA256
   ============================================================ */

int ace_hmac_sha256(uint8_t *mac,
                    const uint8_t *key,
                    size_t key_len,
                    const uint8_t *data,
                    size_t data_len)
{
    struct tc_hmac_state_struct state;

    if (mac == NULL || key == NULL)
        return TC_CRYPTO_FAIL;

    if (data == NULL && data_len != 0)
        return TC_CRYPTO_FAIL;

    if (tc_hmac_set_key(&state, key, (unsigned int)key_len)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (tc_hmac_init(&state) != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (data_len != 0) {
        if (tc_hmac_update(&state,
                           data,
                           (unsigned int)data_len)
            != TC_CRYPTO_SUCCESS)
            return TC_CRYPTO_FAIL;
    }

    return tc_hmac_final(mac,
                         TC_SHA256_DIGEST_SIZE,
                         &state);
}


/* ============================================================
   CMAC
   ============================================================ */

int ace_cmac(uint8_t *mac,
             const uint8_t *key,
             const uint8_t *data,
             size_t data_len)
{
    struct tc_aes_key_sched_struct sched;
    struct tc_cmac_struct state;

    if (mac == NULL || key == NULL)
        return TC_CRYPTO_FAIL;

    if (data == NULL && data_len != 0)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_encrypt_key(&sched, key)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (tc_cmac_setup(&state, key, &sched)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (tc_cmac_init(&state)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (data_len != 0) {
        if (tc_cmac_update(&state,
                           data,
                           data_len)
            != TC_CRYPTO_SUCCESS)
            return TC_CRYPTO_FAIL;
    }

    return tc_cmac_final(mac, &state);
}


/* ============================================================
   CTR
   ============================================================ */

int ace_ctr(uint8_t *out,
            const uint8_t *in,
            size_t len,
            const uint8_t *key,
            uint8_t *ctr)
{
    struct tc_aes_key_sched_struct sched;

    if (out == NULL ||
        in == NULL ||
        key == NULL ||
        ctr == NULL ||
        len == 0)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_encrypt_key(&sched, key)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    return tc_ctr_mode(out,
                       (unsigned int)len,
                       in,
                       (unsigned int)len,
                       ctr,
                       &sched);
}


/* ============================================================
   CBC
   ============================================================ */

int ace_cbc_encrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *in,
                    size_t in_len,
                    const uint8_t *iv,
                    const uint8_t *key)
{
    struct tc_aes_key_sched_struct sched;

    if (out == NULL ||
        in == NULL ||
        iv == NULL ||
        key == NULL)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_encrypt_key(&sched, key)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    return tc_cbc_mode_encrypt(out,
                               (unsigned int)out_len,
                               in,
                               (unsigned int)in_len,
                               iv,
                               &sched);
}


/*
 * TinyCrypt CBC decryption requires the IV to be contiguous
 * immediately before the ciphertext.
 *
 * Therefore `in` must point to:
 *
 *     [16-byte IV][ciphertext]
 *
 * and out_len must be in_len - 16.
 */
int ace_cbc_decrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *in,
                    size_t in_len,
                    const uint8_t *key)
{
    struct tc_aes_key_sched_struct sched;

    if (out == NULL ||
        in == NULL ||
        key == NULL)
        return TC_CRYPTO_FAIL;

    if (in_len < TC_AES_BLOCK_SIZE)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_decrypt_key(&sched, key)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    return tc_cbc_mode_decrypt(out,
                               (unsigned int)out_len,
                               in,
                               (unsigned int)in_len,
                               in,
                               &sched);
}


/* ============================================================
   CCM
   ============================================================ */

int ace_ccm_encrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *aad,
                    size_t aad_len,
                    const uint8_t *plaintext,
                    size_t plaintext_len,
                    uint8_t *nonce,
                    size_t nonce_len,
                    unsigned int tag_len,
                    const uint8_t *key)
{
    struct tc_aes_key_sched_struct sched;
    struct tc_ccm_mode_struct state;

    if (out == NULL ||
        nonce == NULL ||
        key == NULL)
        return TC_CRYPTO_FAIL;

    if (plaintext == NULL && plaintext_len != 0)
        return TC_CRYPTO_FAIL;

    if (aad == NULL && aad_len != 0)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_encrypt_key(&sched, key)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (tc_ccm_config(&state,
                      &sched,
                      nonce,
                      (unsigned int)nonce_len,
                      tag_len)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    return tc_ccm_generation_encryption(
        out,
        (unsigned int)out_len,
        aad,
        (unsigned int)aad_len,
        plaintext,
        (unsigned int)plaintext_len,
        &state
    );
}


int ace_ccm_decrypt(uint8_t *out,
                    size_t out_len,
                    const uint8_t *aad,
                    size_t aad_len,
                    const uint8_t *ciphertext,
                    size_t ciphertext_len,
                    uint8_t *nonce,
                    size_t nonce_len,
                    unsigned int tag_len,
                    const uint8_t *key)
{
    struct tc_aes_key_sched_struct sched;
    struct tc_ccm_mode_struct state;

    if (out == NULL ||
        ciphertext == NULL ||
        nonce == NULL ||
        key == NULL)
        return TC_CRYPTO_FAIL;

    if (aad == NULL && aad_len != 0)
        return TC_CRYPTO_FAIL;

    if (tc_aes128_set_decrypt_key(&sched, key)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    /*
     * TinyCrypt CCM authentication/decryption uses the AES
     * key schedule configured for the CCM context.
     */
    if (tc_ccm_config(&state,
                      &sched,
                      nonce,
                      (unsigned int)nonce_len,
                      tag_len)
        != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    return tc_ccm_decryption_verification(
        out,
        (unsigned int)out_len,
        aad,
        (unsigned int)aad_len,
        ciphertext,
        (unsigned int)ciphertext_len,
        &state
    );
}


/* ============================================================
   HMAC-PRNG
   ============================================================ */

int ace_hmac_prng_init(TCHmacPrng_t ctx,
                       const uint8_t *personalization,
                       size_t personalization_len)
{
    if (ctx == NULL ||
        personalization == NULL)
        return TC_CRYPTO_FAIL;

    return tc_hmac_prng_init(
        ctx,
        personalization,
        (unsigned int)personalization_len
    );
}


int ace_hmac_prng_reseed(TCHmacPrng_t ctx,
                         const uint8_t *seed,
                         size_t seed_len,
                         const uint8_t *additional_input,
                         size_t additional_input_len)
{
    if (ctx == NULL || seed == NULL)
        return TC_CRYPTO_FAIL;

    return tc_hmac_prng_reseed(
        ctx,
        seed,
        (unsigned int)seed_len,
        additional_input,
        (unsigned int)additional_input_len
    );
}


int ace_hmac_prng_generate(TCHmacPrng_t ctx,
                           uint8_t *out,
                           size_t len)
{
    if (ctx == NULL || out == NULL || len == 0)
        return TC_CRYPTO_FAIL;

    return tc_hmac_prng_generate(
        out,
        (unsigned int)len,
        ctx
    );
}


/* ============================================================
   CTR-PRNG
   ============================================================ */

int ace_ctr_prng_init(TCCtrPrng_t *ctx,
                      const uint8_t *entropy,
                      size_t entropy_len,
                      const uint8_t *personalization,
                      size_t personalization_len)
{
    if (ctx == NULL || entropy == NULL)
        return TC_CRYPTO_FAIL;

    return tc_ctr_prng_init(
        ctx,
        entropy,
        (unsigned int)entropy_len,
        personalization,
        (unsigned int)personalization_len
    );
}


int ace_ctr_prng_reseed(TCCtrPrng_t *ctx,
                        const uint8_t *entropy,
                        size_t entropy_len,
                        const uint8_t *additional_input,
                        size_t additional_input_len)
{
    if (ctx == NULL || entropy == NULL)
        return TC_CRYPTO_FAIL;

    return tc_ctr_prng_reseed(
        ctx,
        entropy,
        (unsigned int)entropy_len,
        additional_input,
        (unsigned int)additional_input_len
    );
}


int ace_ctr_prng_generate(TCCtrPrng_t *ctx,
                          const uint8_t *additional_input,
                          size_t additional_input_len,
                          uint8_t *out,
                          size_t len)
{
    if (ctx == NULL || out == NULL || len == 0)
        return TC_CRYPTO_FAIL;

    return tc_ctr_prng_generate(
        ctx,
        additional_input,
        (unsigned int)additional_input_len,
        out,
        (unsigned int)len
    );
}


void ace_ctr_prng_uninstantiate(TCCtrPrng_t *ctx)
{
    if (ctx != NULL)
        tc_ctr_prng_uninstantiate(ctx);
}


/* ============================================================
   ECC
   ============================================================ */

void ace_ecc_set_rng(uECC_RNG_Function rng)
{
    uECC_set_rng(rng);
}


uECC_RNG_Function ace_ecc_get_rng(void)
{
    return uECC_get_rng();
}


uECC_Curve ace_ecc_secp256r1(void)
{
    return uECC_secp256r1();
}


int ace_ecc_private_key_size(uECC_Curve curve)
{
    return uECC_curve_private_key_size(curve);
}


int ace_ecc_public_key_size(uECC_Curve curve)
{
    return uECC_curve_public_key_size(curve);
}


int ace_ecc_make_key(uint8_t *public_key,
                     uint8_t *private_key,
                     uECC_Curve curve)
{
    return uECC_make_key(
        public_key,
        private_key,
        curve
    );
}


int ace_ecc_compute_public_key(const uint8_t *private_key,
                               uint8_t *public_key,
                               uECC_Curve curve)
{
    return uECC_compute_public_key(
        private_key,
        public_key,
        curve
    );
}


int ace_ecc_validate_point(const uECC_word_t *point,
                           uECC_Curve curve)
{
    return uECC_valid_point(point, curve);
}


int ace_ecc_validate_public_key(const uint8_t *public_key,
                                uECC_Curve curve)
{
    return uECC_valid_public_key(public_key, curve);
}


/* ============================================================
   ECDH
   ============================================================ */

int ace_ecdh_shared_secret(const uint8_t *public_key,
                           const uint8_t *private_key,
                           uint8_t *secret,
                           uECC_Curve curve)
{
    return uECC_shared_secret(
        public_key,
        private_key,
        secret,
        curve
    );
}


/* ============================================================
   ECDSA
   ============================================================ */

int ace_ecdsa_sign(const uint8_t *private_key,
                   const uint8_t *hash,
                   unsigned int hash_len,
                   uint8_t *signature,
                   uECC_Curve curve)
{
    return uECC_sign(
        private_key,
        hash,
        hash_len,
        signature,
        curve
    );
}


int ace_ecdsa_verify(const uint8_t *public_key,
                     const uint8_t *hash,
                     unsigned int hash_len,
                     const uint8_t *signature,
                     uECC_Curve curve)
{
    return uECC_verify(
        public_key,
        hash,
        hash_len,
        signature,
        curve
    );
}


/* ============================================================
   Constant-time utility
   ============================================================ */

int ace_crypto_compare(const uint8_t *a,
                       const uint8_t *b,
                       size_t len)
{
    if (a == NULL || b == NULL)
        return 1;

    return _compare(a, b, len);
}

// RSA

static inline void ace_rsa_generate_key_pair(
    bigint p,
    bigint q,
    bigint *n,
    bigint *t,
    bigint *e,
    bigint *d
) {
    rsa_generate_key_pair(p, q, n, t, e, d);
}

static inline bigint *ace_rsa_encrypt_text(
    char *message,
    bigint e,
    bigint n
) {
    return rsa_encrypt_text(message, e, n);
}

static inline char *ace_rsa_decrypt_text(
    bigint *encrypted,
    int length,
    bigint d,
    bigint n
) {
    return rsa_decrypt_text(encrypted, length, d, n);
}

/* ============================================================
 * BIGINT
 *
 * Add the remaining bigint_* wrappers here using the exact
 * signatures from bigint.h.
 * ============================================================ */

static inline bigint ace_rsa_bigint_from_string(const char *str)
{
    return bigint_from_string(str);
}

static inline void ace_rsa_bigint_delete(bigint n)
{
    bigint_delete(n);
}

static inline bool ace_rsa_bigint_eqzero(bigint n)
{
    return bigint_eqzero(n);
}

static inline bigint ace_rsa_bigint_mod(bigint a, bigint b)
{
    return bigint_mod(a, b);
}

void ace_rsa_init(
    const char *p1_str,
    const char *p2_str,
    bigint *n,
    bigint *t,
    bigint *e,
    bigint *d
)
{
    bigint p1 = bigint_from_string(p1_str);
    bigint p2 = bigint_from_string(p2_str);

    rsa_generate_key_pair(p1, p2, n, t, e, d);

    bigint_delete(p1);
    bigint_delete(p2);
}

void ace_rsa_enc_print(bigint *encrypted, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        bigint_print(encrypted[i]);
        kprintf(" ");
    }
}

// SHA

int ace_sha512(uint8_t *digest,
               const uint8_t *data,
               size_t len)
{
    struct tc_sha512_state_struct state;

    if (digest == NULL)
        return TC_CRYPTO_FAIL;

    if (data == NULL && len != 0)
        return TC_CRYPTO_FAIL;

    if (tc_sha512_init(&state) != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (len != 0) {
        if (tc_sha512_update(&state, data, len) != TC_CRYPTO_SUCCESS)
            return TC_CRYPTO_FAIL;
    }

    return tc_sha512_final(digest, &state);
}


int ace_sha384(uint8_t *digest,
               const uint8_t *data,
               size_t len)
{
    struct tc_sha512_state_struct state;

    if (digest == NULL)
        return TC_CRYPTO_FAIL;

    if (data == NULL && len != 0)
        return TC_CRYPTO_FAIL;

    if (tc_sha384_init(&state) != TC_CRYPTO_SUCCESS)
        return TC_CRYPTO_FAIL;

    if (len != 0) {
        if (tc_sha512_update(&state, data, len) != TC_CRYPTO_SUCCESS)
            return TC_CRYPTO_FAIL;
    }

    return tc_sha384_final(digest, &state);
}
