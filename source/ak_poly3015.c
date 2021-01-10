//
// Created by r.anchugov on 09.01.2021.
//
#include <libakrypt.h>
#include <stdlib.h>
#include <stdio.h>

void print_mpzn(ak_uint128 to_print){
    const char* res = ak_mpzn_to_hexstr(to_print.q, 2);
    printf("\nBIG NUMBER IS: ");
    for (int i = 0; i < strlen(res); ++i) {
        printf("%c", res[i]);
    }
    printf("\n");
}

ak_uint128 clamp(ak_uint128 to_clamp) {
    ak_uint128 clamper;
    ak_mpzn_set_hexstr(clamper.q, 2, "0ffffffc0ffffffc0ffffffc0fffffff");
    print_mpzn(clamper);
    print_mpzn(to_clamp);
    to_clamp.q[0] &= clamper.q[0];
    to_clamp.q[1] &= clamper.q[1];
    print_mpzn(to_clamp);
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
    ak_uint128 r, s;
    s = le_bytes_to_num(key + 32, 32, ak_true);
    r = le_bytes_to_num(key, 32, ak_true);
    ak_uint128 clamp_result = clamp(r);
    const char *test = ak_mpzn_to_hexstr(clamp_result.q, 2);
    for (int i = 0; i < strlen(test); ++i) {
        printf("%c", test[i]);
    }
}
