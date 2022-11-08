#include "Enclave_t.h"
#include "sgx_trts.h"
#include <stdlib.h>
#include <stdarg.h>
#include <cstring>
#include <stdio.h>


void ecall_enter_enclave(){
   int* ptr = NULL;
   ocall_return_an_untrusted_pointer(&ptr);
   *ptr = 100; // ==> leak
}