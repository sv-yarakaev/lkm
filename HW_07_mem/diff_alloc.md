Память на машине:
```bash
free -m
               total        used        free      shared  buff/cache   available
Mem:            3893         951        1946           9        1284        2941
Swap:           3892          62        3830
```
get_page использование __get_free_pages(GFP_KERNEL, order);

```bash
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 16384 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 32768 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 32768 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 65536 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 65536 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 131072 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 131072 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 262144 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 262144 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 524288 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 524288 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 1048576 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 1048576 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 2097152 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 2097152 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 4194304 byte
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: SUCCESS
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 4194304 byte, 0 ms, type: physical contiguous pages
Oct 06 14:36:04 build.minimax.su kernel: ex_get_page: get_page: 8388608 byte
```

Использование  kmem_cache_create
```bash
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 1024 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 1024 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 2048 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 2048 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 4096 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 4096 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 8192 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 8192 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 16384 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 16384 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 32768 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 32768 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 65536 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 65536 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 131072 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 131072 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 262144 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 262144 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 524288 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 524288 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 1048576 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 1048576 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 2097152 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 2097152 byte, 0 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 4194304 byte
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: SUCCESS
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 4194304 byte, 1 ms, type: object cached physical
Oct 06 15:04:41 build.minimax.su kernel: kmem_cache: 8388608 byte
```
Использование kmem_cache_create
```bash
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 1024 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 1024 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 2048 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 2048 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 4096 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 4096 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 8192 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 8192 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 16384 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 16384 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 32768 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 32768 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 65536 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 65536 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 131072 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 131072 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 262144 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 262144 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 524288 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 524288 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 1048576 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 1048576 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 2097152 byte
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 2097152 byte, 0 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 4194304 byte
Oct 06 15:44:53 build.minimax.su sudo[1826783]: pam_unix(sudo:session): session opened for user root(uid=0) by stv(uid=1000)
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: SUCCESS
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 4194304 byte, 1 ms, type: pooled object cached
Oct 06 15:44:53 build.minimax.su kernel: ex_mempool: mempool: 8388608 byte
```

Использование kmalloc 
```bash
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 1024 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 1024 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 2048 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 2048 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 4096 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 4096 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 8192 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 8192 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 16384 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 16384 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 32768 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 32768 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 65536 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 65536 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 131072 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 131072 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 262144 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 262144 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 524288 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 524288 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 1048576 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 1048576 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 2097152 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 2097152 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 4194304 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: SUCCESS
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 4194304 byte, 0 ms, type: physical contiguous
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: 8388608 byte
Oct 06 16:15:22 build.minimax.su kernel: kmalloc: FAIL, err_msg = out of memory
```

Использование vmalloc
```bash
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 1024 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 1024 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 2048 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 2048 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 4096 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 4096 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 8192 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 8192 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 16384 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 16384 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 32768 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 32768 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 65536 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 65536 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 131072 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 131072 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 262144 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 262144 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 524288 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 524288 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 1048576 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 1048576 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 2097152 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 2097152 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 4194304 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 4194304 byte, 0 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 8388608 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 8388608 byte, 1 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 16777216 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 16777216 byte, 2 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 33554432 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 33554432 byte, 6 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 67108864 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 33554432 byte, 6 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 67108864 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 67108864 byte, 11 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 134217728 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 134217728 byte, 22 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 268435456 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 268435456 byte, 48 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 536870912 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 536870912 byte, 96 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 1073741824 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 1073741824 byte, 143 ms, type: virtual non-contiguous
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 2147483648 byte
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: SUCCESS
Oct 06 16:23:09 build.minimax.su kernel: ex_vmalloc: vmalloc: 2147483648 byte, 317 ms, type: virtual non-contiguous

```


Фактически все аллокаторы ведут себя ожидаемо в пределах выделенной физической памяти ядра. Выделяют пока не упрутся в 4Mb.  Конечно, кроме vmalloc, он уперся в 4294967296 (4Gb)
Отличия:
- kmalloc/get_page: Самые быстрые для непрерывной памяти, но ограничены максимальным размером.
- vmalloc: Поддерживает наибольшие размеры, но медленнее(по логу видно).
- kmem_cache/mempool: Добавляют кэширование/пулирование для эффективности/надежности; 
- mempool оптимизирован для избежания нехватки памяти.

