#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

int main(){
    //allocate memory on heap
    int *arr = malloc(1000*sizeof(int));
    if(arr == NULL){
        perror("malloc failed");
        return 1;
    }
    // touch memory (force allocation)
    for(int i=0; i<1000; i++){
        arr[i] = i;
    }
    // get virtual address
    uintptr_t virtual_address= (uintptr_t)&arr[0];
    int page_size = getpagesize();

    printf("Virtual address: %p\n", (void*)virtual_address);
    printf("Page size: %d\n", page_size);

    uintptr_t page_number = virtual_address / page_size;
    uintptr_t offset = virtual_address%page_size;

    printf("Page number: %lu\n", (unsigned long) page_number);
    printf("Offset: %lu\n", (unsigned long) offset);
    // open pagemap
    int fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd ==-1){
        perror("Error opening pagemap");
        free(arr);
        return 1;
    }
    // go to correct entry
    // 8 is the size of uint64_t
    off_t file_offset = page_number*8;
    if (lseek(fd, file_offset, SEEK_SET) == -1){
        perror("lseek failed");
        close(fd);
        free(arr);
        return 1;
    }
    // read entry
    uint64_t entry;
    if (read(fd, &entry, sizeof(entry)) != sizeof(entry)){
        perror("read failed");
        close(fd);
        free(arr);
        return 1;
    }
    close(fd);

    // extract pfn
    uintptr_t pfn = entry & ((1ULL << 55) - 1);

    printf("PFN: %lu\n", (unsigned long)pfn);

    uintptr_t physical_address = (pfn * page_size) + offset;
    printf("Physical address: %lu\n", (unsigned long) physical_address);

    free(arr);

    return 0;
};

