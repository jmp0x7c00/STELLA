#include "App.h"
#include "sgx_urts.h"
#include "Enclave_u.h"
#include <stdio.h>
#include "sgx_capable.h"
#include "sgx_uae_service.h"
#include <stdlib.h>
#include <stdio.h>

sgx_enclave_id_t create_enclave(){
    sgx_enclave_id_t enclaveId = NULL;
    sgx_status_t ret = SGX_SUCCESS;
    sgx_launch_token_t token = {0};
    int updated;
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &enclaveId, NULL);
    if(ret != SGX_SUCCESS){
        printf("Error %#x: cannot create enclave\n", ret);
        exit(-1);
    }
    return enclaveId;
}

 void ocall_in_pointer_leak(int* ptr){
     printf("%d\n",*ptr);
 }

int main(){
        ecall_demo(create_enclave());
        return 0;
}
