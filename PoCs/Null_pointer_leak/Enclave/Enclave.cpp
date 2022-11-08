#include "Enclave_t.h"
#include "sgx_trts.h"
#include <cstring>

void ecall_demo(){
    void *dst = NULL; // Programmer forgot to initialize it correctly
    char src[9] = {'p','a','s','s','w','o','r','d','\0'};
    memcpy(dst,src,sizeof(src)); // information leak
}