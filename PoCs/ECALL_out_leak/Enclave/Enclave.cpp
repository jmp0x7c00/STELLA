#include "Enclave_t.h"
#include "sgx_trts.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

void ecall_out_pointer_leak(int* out_ptr){
	int secret = 100;
	*out_ptr = secret;// ==> leak
}


