/*
 Small tool to add empty fat table to the image



*/

#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int patch(char *mem)
{
    uint16_t BPB_BytsPerSec = *(uint16_t *)(&mem[11]);
    uint16_t BPB_RsvdSecCnt = *(uint16_t *)(&mem[14]);

    uint32_t fat_start = BPB_RsvdSecCnt * BPB_BytsPerSec;

    unsigned char sample_fat[] = {
        0xF8,
        0xFF,
        0xFF,
        0x0F,
        0xFF,
        0xFF,
        0xFF,
        0x0F,
        0xF8,
        0xFF,
        0xFF,
        0x0F,
    };

    memcpy(&mem[fat_start], sample_fat, sizeof(sample_fat));
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Provide a file as first argument!\n");
        return 1;
    }

    // | O_CREAT | O_TRUNC
    int fd = open(argv[1], O_RDWR, (mode_t)0600);

    if (fd == -1)
    {
        perror("Error opening file for writing");
        return 1;
    }
    struct stat sb;
    fstat(fd, &sb);
    printf("Size: %lu\n", (uint64_t)sb.st_size);

    char *mem = mmap(NULL, sb.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED)
    {
        perror("Error mmap file for writing");
        return 1;
    }

    int patch_status = patch(mem);
    if (patch_status)
    {
        return patch_status;
    }

    if (munmap(mem, sb.st_size) == -1)
    {
        perror("Error un-mmapping the file");
        return 1;
    }

    close(fd);
    return 0;
}