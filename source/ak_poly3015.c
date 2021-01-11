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
    print_mpzn(clamper.q, 2);
    print_mpzn(to_clamp.q, 2);
    to_clamp.q[0] &= clamper.q[0];
    to_clamp.q[1] &= clamper.q[1];
    print_mpzn(to_clamp.q, 2);
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

void poly1305(char *msg, char *key) {
    ak_uint64 r[4], s[4];
    ak_uint128 s_num = le_bytes_to_num(key + 32, 32, ak_true);
    ak_uint128 r_num = le_bytes_to_num(key, 32, ak_true);
    r_num = clamp(r_num);
    r[0] = r_num.q[0];
    r[1] = r_num.q[1];
    r[2] = r[3] = 0;
    s[0] = s[1] = s[2] = s[3] = 0;

    ak_uint64 p[4];
    ak_uint64 a[4];
    a[0] = a[1] = a[2] = a[3] = 0;
    ak_mpzn_set_hexstr(p, 4, "3fffffffffffffffffffffffffffffffb");

    for (int i = 1; i <= ceil(strlen(msg) / 16.0) + 1; ++i) {
        unsigned long count = (strlen(msg) - (i - 1) * 16 > 16) ? 16 : strlen(msg) - (i - 1) * 16;
        ak_uint128 num = le_bytes_to_num(&msg[(i - 1) * 16], count, ak_false);
        ak_uint64 n[4];
        n[0] = num.q[0];
        n[1] = num.q[1];
        n[2] = n[3] = 0;
        printf("----------\n\n");
        printf("Acc\n");
        print_mpzn(a, 4);
        printf("Block %i\n", i);
        print_mpzn(n, 4);

        //2^(count*8)
        ak_uint128 two;
        two.q[0] = two.q[1] = 0;
        ak_uint64 result[4];
        ak_mpzn_set_hexstr(two.q, 2, "2");
        ak_mpzn_set_hexstr(result, 4, "2");
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

        ak_uint64 dop[4];
        dop[0] = dop[1] = dop[2] = dop[3] = 0;

        //dop = (r*a)
        printf("(Acc+Block) * r \n");
        ak_mpzn_mul(dop, r, a, 4);
        print_mpzn(dop, 4);


        //TODO: a = dop % p
        int cmp_res = ak_mpzn_cmp(dop, p, 4);

        printf("P\n");
        print_mpzn(dop, 4);
        ak_mpzn_sub(dop, dop, dop, 4);
        print_mpzn(dop, 4);

        printf("\\perscent a\n");
        print_mpzn(dop, 4);
    }

    //TODO: a += s
}
