#include <iostream>
#include <gcrypt.h>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <cmath>

void print_mpi(gcry_mpi_t value) {
    unsigned char *buffer;
    gcry_error_t err = gcry_mpi_aprint(GCRYMPI_FMT_HEX, &buffer, NULL, value);
    if (err != 0) {
        printf(" error: %d\n", err);
    }
    printf("MPI %s\n", buffer);
    gcry_free(buffer);
}

void reverse_bytes(std::string &str) {
    int n = str.length();
    int i = n % 2 == 0 ? 0 : 1;
    for (i = 0; i < n / 2 - 1; i += 2) {
        std::swap(str[i], str[n - i - 2]);
        std::swap(str[i + 1], str[n - i - 1]);
    }
    str = str.substr(0, 32);
}

gcry_mpi_t le_bytes_to_num(char *bytes, int n) {
    std::stringstream ss;
    for (int i = 0; i < n; i++) {
        ss << std::setfill('0') << std::setw(sizeof(char) * 2) << std::hex << (int) bytes[i];
    }
    gcry_mpi_t num = gcry_mpi_new(0);
    std::string result_string = ss.str();
    reverse_bytes(result_string);
    gcry_mpi_scan(&num, GCRYMPI_FMT_HEX, result_string.c_str(), 0, NULL);
    return num;
}

std::string num_to_le_bytes(gcry_mpi_t num) {
    unsigned char *buffer;
    gcry_error_t err = gcry_mpi_aprint(GCRYMPI_FMT_HEX, &buffer, NULL, num);
    if (err != 0) {
        printf(" error: %d\n", err);
    }
    int i = 0;
    std::stringstream ss;
    while (buffer[i] != '\0') {
        ss << buffer[i];
        i++;
    }
    std::string result = ss.str();
    reverse_bytes(result);
    return result;
}

gcry_mpi_t clamp(gcry_mpi_t to_clamp) {
    gcry_mpi_t clamper, stub;
    gcry_mpi_scan(&clamper, GCRYMPI_FMT_HEX, "0ffffffc0ffffffc0ffffffc0fffffff", 0, NULL);
    gcry_mpi_scan(&stub, GCRYMPI_FMT_HEX, "1", 0, NULL);
    gcry_mpi_mulm(to_clamp, to_clamp, stub, clamper);
    return to_clamp;
}

void poly1305(char *msg, char *key, bool is_test) {
    gcry_mpi_t r, s;
    if (is_test) {
        r = gcry_mpi_new(0);
        s = gcry_mpi_new(0);
        gcry_mpi_scan(&s, GCRYMPI_FMT_HEX, "1bf54941aff6bf4afdb20dfb8a800301", 0, NULL);
        gcry_mpi_scan(&r, GCRYMPI_FMT_HEX, "806d5400e52447c036d555408bed685", 0, NULL);
        char test_msg[35] = "Cryptographic Forum Research Group";
        msg = test_msg;
    } else {
        r = le_bytes_to_num(key, 16);
        s = le_bytes_to_num(&key[16], 16);
    }
    clamp(r);
    gcry_mpi_t a, p;
    gcry_mpi_scan(&a, GCRYMPI_FMT_HEX, "0", 0, NULL);
    gcry_mpi_scan(&p, GCRYMPI_FMT_HEX, "3fffffffffffffffffffffffffffffffb", 0, NULL);


    for (int i = 1; i <= std::ceil(strlen(msg) / 16) + 1; ++i) {
        std::cout << "Block " << i << std::endl;
        int count = (strlen(msg) - (i - 1) * 16 > 16) ? 16 : strlen(msg) - (i - 1) * 16;
        std::cout << "Block length: " << count << std::endl;
        gcry_mpi_t n = le_bytes_to_num(&msg[(i - 1) * 16], count);
        print_mpi(n);

        //2^(count*8)
        gcry_mpi_t two = gcry_mpi_new(0);
        gcry_mpi_t result = gcry_mpi_new(0);
        gcry_mpi_scan(&two, GCRYMPI_FMT_HEX, "2", 0, NULL);
        gcry_mpi_scan(&result, GCRYMPI_FMT_HEX, "2", 0, NULL);
        for (int j = 1; j < count * 8; j++) {
            gcry_mpi_mul(result, result, two);
        }
        gcry_mpi_add(n, n, result);
        std::cout << "After addition of 2^" << count << ": ";
        print_mpi(n);
        //a += n
        gcry_mpi_add(a, a, n);
        std::cout << "Acc + block: ";
        print_mpi(a);
        gcry_mpi_t dop = gcry_mpi_new(0);
        //dop = (r*a)
        gcry_mpi_mul(dop, r, a);
        std::cout << "(Acc + block) * r";
        print_mpi(dop);
        //a = (r * a) % p
        gcry_mpi_mod(a, dop, p);
        std::cout << "Acc" << std::endl;
        print_mpi(a);
    }
    //a += s
    gcry_mpi_add(a, a, s);
    std::string result = num_to_le_bytes(a);
    std::cout << "TAG is: " << result;
}

int main() {
    poly1305(NULL, NULL, true);
    return 0;
}

