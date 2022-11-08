#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>

int main(){
    int i;
    void *start = NULL;
    size_t page_size = getpagesize();
    while(1){
        void *addr = mmap(start, page_size, PROT_READ|PROT_WRITE, \
        MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);
        if(addr != MAP_FAILED) {
            printf("lowest mapable address: %p\n", start);
            break;
        }
        start += page_size;
    }
    return 0;
}
