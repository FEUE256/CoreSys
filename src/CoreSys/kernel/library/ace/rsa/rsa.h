#ifndef RSA_H
#define RSA_H

#include "bigint.h"

#include <drivers/page/main.h>
#include <drivers/serial/main.h>

bigint gcd(bigint a, bigint b) {
    if (bigint_eqzero(b)) {
        return a;
    }
    return gcd(b, bigint_mod(a, b));
}

bigint lcm(bigint a, bigint b) {
    bigint product = bigint_mul(a, b);
    bigint greatest_common_divisor = gcd(a, b);
    return bigint_div(product, greatest_common_divisor);
}

bigint totient(bigint p1, bigint p2) {
    bigint p1_minus_one = bigint_sub(p1, bigint_from_int(1));
    bigint p2_minus_one = bigint_sub(p2, bigint_from_int(1));
    return lcm(p1_minus_one, p2_minus_one);
}

bigint rsa_encrypt(bigint message, bigint e, bigint n) {
    return bigint_fast_pow(message, e, n);
}

bigint rsa_decrypt(bigint message, bigint d, bigint n) {
    return bigint_fast_pow(message, d, n);
}

bigint *rsa_encrypt_text(char *message, bigint e, bigint n) {
    bigint *encrypted = kmalloc(strlen(message) * sizeof(bigint));
    size_t msg_len = strlen(message);
    for (size_t i = 0; i < msg_len; i++) {
        // bigint m = bigint_from_int(message[i]);
        bigint m = bigint_from_int(message[i] + i);
        encrypted[i] = rsa_encrypt(m, e, n);
        bigint_delete(m);
    }
    return encrypted;
}

char *rsa_decrypt_text(bigint *encrypted, int length, bigint d, bigint n) {
    char *message = kmalloc(length + 1);
    for (int i = 0; i < length; i++) {
        bigint m = rsa_decrypt(encrypted[i], d, n);
        message[i] = (char)bigint_to_int(m) - i;
        // message[i] = (char)bigint_to_int(m);
        bigint_delete(m);
    }
    message[length] = '\0';
    return message;
}

void rsa_generate_key_pair(bigint p1, bigint p2, bigint *n, bigint *t, bigint *e, bigint *private_key) {
    // Calculate the modulus
    *n = bigint_mul(p1, p2);
    // Calculate the totient
    *t = totient(p1, p2);

    // Choose an E value that is relatively prime to (p1 - 1)(p2 - 1)
    // Choose the largest prime less than 65537
    bigint public_key = bigint_from_int(65537);
    if (!bigint_lt(public_key, *t))
        public_key = bigint_from_int(17);
    if (!bigint_lt(public_key, *t))
        public_key = bigint_from_int(5);
    if (!bigint_lt(public_key, *t))
        public_key = bigint_from_int(3);
    if (!bigint_lt(public_key, *t))
        public_key = bigint_from_int(2);

    *e = public_key;

    // Calculate the private key
    *private_key = bigint_modinv(public_key, *t);
}
#endif
