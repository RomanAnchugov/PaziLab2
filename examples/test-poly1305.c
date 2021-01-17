//
// Created by r.anchugov on 09.01.2021.
//
#include<libakrypt.h>

int main() {
    char *test_tag = "a8061dc1305136c6c22b8baf0c0127a9";
    const char* tag = poly1305("Cryptographic Forum Research Group", "85d6be7857556d337f4452fe42d506a80103808afb0db2fd4abff6af4149f51b");
    if(strcmp(test_tag, tag) == 0){
        printf("\n\nPOLY1305 TEST PASSED");
    }else{
        printf("\n\nPOLY1305 TEST FAILED");
    }
}
