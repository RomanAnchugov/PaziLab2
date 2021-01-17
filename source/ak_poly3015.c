//
// Created by r.anchugov on 09.01.2021.
//
#include <libakrypt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void print_mpzn(ak_uint64 *to_print, int size) {
    const char *res = ak_mpzn_to_hexstr(to_print, size);
    printf("BIG NUMBER IS: ");
    for (int i = 0; i < strlen(res); ++i) {
        printf("%c", res[i]);
    }
    printf("\n");
}

ak_uint128 clamp(ak_uint128 to_clamp) {
    ak_uint128 clamper;
    ak_mpzn_set_hexstr(clamper.q, 2, "0ffffffc0ffffffc0ffffffc0fffffff");
    to_clamp.q[0] &= clamper.q[0];
    to_clamp.q[1] &= clamper.q[1];
    return to_clamp;
}

ak_uint128 le_bytes_to_num(char *bytes, int length, bool_t is_hex) {
    if (is_hex == ak_false) {
        char hex_string[length * 2 + 1];
        //Convert bytes to hex representation
        for (int i = 0; i < length; i++) {
            sprintf(hex_string + i * 2, "%02X", bytes[i]);
        }
        char result[length * 2 + 1];
        //Reverse octets, due to le
        for (int i = length * 2 - 1; i >= 1; i -= 2) {
            sprintf(result + (length * 2 - 1 - i), "%c%c", hex_string[i - 1], hex_string[i]);
        }
        ak_uint128 num_result;
        ak_mpzn_set_hexstr(num_result.q, 2, result);
        return num_result;
    } else {
        char result[length];
        for (int i = length - 1; i >= 1; i -= 2) {
            sprintf(result + (length - 1 - i), "%c%c", bytes[i - 1], bytes[i]);
        }
        printf("HEX RESULT:");
        for (int i = 0; i < strlen(result); ++i) {
            printf("%c", result[i]);
        }
        printf("\n");
        ak_uint128 num_result;
        ak_mpzn_set_hexstr(num_result.q, 2, result);
        return num_result;
    }
}

const char *num_to_16le_bytes(ak_uint64 *num, size_t size) {
    const char *num_str = ak_mpzn_to_hexstr(num, size);
    char *result = malloc(strlen(num_str));
    for (int i = strlen(num_str) - 1; i >= 1; i -= 2) {
        sprintf(result + (strlen(num_str) - i - 1), "%c%c", num_str[i - 1], num_str[i]);
    }
    return result;
}

void shift_left(ak_uint64 *r, const ak_uint64 *from, size_t shift_count) {
    for (size_t i = 0; i < shift_count / 64; ++i)
        r[i] = 0;
    for (size_t i = shift_count / 64; i < 8; ++i)
        r[i] = from[i - shift_count / 64];
    shift_count %= 64;
    if (shift_count == 0)
        return;
    r[7] <<= shift_count;
    for (int i = 6; i >= 0; --i) {
        r[i + 1] |= (r[i] >> (64 - shift_count));
        r[i] <<= shift_count;
    }
}

void rem(ak_uint64 *r, ak_uint64 *u, ak_uint64 *p) {
    const size_t size = ak_mpzn512_size;
    ak_mpzn512 shifted_p;
    ak_mpzn512 from;
    ak_mpzn_set(from, u, size);

    for (int i = 256; i >= 0; --i) {
        shift_left(shifted_p, p, i);
        if (ak_mpzn_cmp(shifted_p, from, size) == -1)
            ak_mpzn_sub(from, from, shifted_p, size);
    }

    ak_mpzn_set(r, from, size);
}

const char *poly1305(char *msg, char *key) {
    ak_mpzn512 r = ak_mpzn512_zero;
    ak_mpzn512 s = ak_mpzn512_zero;
    ak_uint128 s_num = le_bytes_to_num(key + 32, 32, ak_true);
    ak_uint128 r_num = le_bytes_to_num(key, 32, ak_true);
    r_num = clamp(r_num);
    r[0] = r_num.q[0];
    r[1] = r_num.q[1];
    s[0] = s_num.q[0];
    s[1] = s_num.q[1];

    ak_mpzn512 p = ak_mpzn512_zero;
    ak_mpzn512 a = ak_mpzn512_zero;
    ak_mpzn_set_hexstr(p, 4, "3fffffffffffffffffffffffffffffffb");

    for (int i = 1; i <= ceil(strlen(msg) / 16.0); ++i) {
        unsigned long count = (strlen(msg) - (i - 1) * 16 > 16) ? 16 : strlen(msg) - (i - 1) * 16;
        ak_uint128 num = le_bytes_to_num(&msg[(i - 1) * 16], count, ak_false);
        ak_mpzn512 n = ak_mpzn512_zero;
        n[0] = num.q[0];
        n[1] = num.q[1];

        printf("----------\n\n");
        printf("Acc\n");
        print_mpzn(a, 4);
        printf("Block %i\n", i);
        print_mpzn(n, 4);

        //2^(count*8)
        ak_uint128 two;
        two.q[0] = two.q[1] = 0;
        ak_mpzn512 result = ak_mpzn512_zero;
        ak_mpzn_set_hexstr(two.q, 2, "2");
        ak_mpzn_set_hexstr(result, ak_mpzn512_size, "2");
        for (int j = 1; j < count * 8; j++) {
            ak_mpzn_mul(result, result, two.q, 2);
        }

        printf("Block + 2^count\n");
        ak_mpzn_add(n, n, result, 4);
        print_mpzn(n, 4);

        //a += n
        printf("Acc + block\n");
        ak_mpzn_add(a, a, n, 4);
        print_mpzn(a, 4);

        ak_mpzn512 dop = ak_mpzn512_zero;

        //dop = (r*a)
        printf("(Acc+Block) * r \n");
        ak_mpzn_mul(dop, r, a, ak_mpzn512_size);
        print_mpzn(dop, ak_mpzn512_size);

        // a = dop % p
        printf("((Acc+Block) * r) mod p\n");
        rem(a, dop, p);
        print_mpzn(a, ak_mpzn512_size);
    }

    printf("a += s\n");
    ak_mpzn_add(a, a, s, ak_mpzn512_size);
    print_mpzn(a, ak_mpzn512_size);

    const char *tag = num_to_16le_bytes(a, 2);
    printf("POLY1305 RESULT TAG: %s", tag);
    return tag;
}
