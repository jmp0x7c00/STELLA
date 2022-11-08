# null pointer dereference leak
* Attacker can modify the low virtual memory addresses to 0.
```shell
sudo su
echo "vm.mmap_min_addr = 0" > /etc/sysctl.d/mmap_min_addr.conf
```

* Attack tempers the untrusted code,using `mmap` function to map page 0.
```c
    void* p = mmap(0, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    printf("mmap result %p (errno %s)\n",p,strerror(errno));
```

* Sensitive data will leak when programmers forget to initialize pointers correctly.
```c
    void *dst = NULL; // Programmer forgot to initialize it correctly
    char src[9] = {'p','a','s','s','w','o','r','d','\0'};
    memcpy(dst,src,sizeof(src)); // information leak
```
* result
```bash
$ ./app
$ password
```

* appendix

    https://wiki.debian.org/mmap_min_addr <br>
    http://richardustc.github.io/2013-05-21-2013-05-21-min-mmap-addr.html


