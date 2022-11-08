#include "Enclave_t.h"
#include "sgx_trts.h"
#include <stdlib.h>
#include <stdarg.h>
#include <cstring>
#include <stdio.h>


void ecall_user_check_pointer_leak(int* ptr){
    int secret = 100;
    *ptr = secret;// ==> leak
}




