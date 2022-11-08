#include "Enclave_t.h"
#include "sgx_trts.h"

void ecall_demo(){
    int secret = 100;
    ocall_in_pointer_leak(&secret);
}




