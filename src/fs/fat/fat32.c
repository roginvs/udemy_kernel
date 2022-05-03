#include "./fat32.h"

#include <stdint.h>
#include <stddef.h>
// #include "memory/memory.h"
#include "string/string.h"
#include "disk/disk.h"
#include "status.h"
#include "disk/streamer.h"
#include "memory/heap/kheap.h"
#include "terminal.h"
#include "kernel.h"

struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_setors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_header_extended_32
{
    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t BPB_Reserved[12];
    uint8_t BS_DrvNum;
    uint8_t BS_Reserved1;
    /** Should be 0x29 */
    uint8_t BS_BootSig;
    uint32_t BS_VolID;
    uint8_t BS_VolLab[11];
    uint8_t BS_FilSysType[8];
} __attribute__((packed));

struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended_32 fat_header_extended_32;
    } shared;
};

int fat32_resolve(struct disk *disk);
void *fat32_open(struct disk *disk, struct path_part *path, FILE_MODE mode);

struct filesystem fat32_fs =
    {
        .resolve = fat32_resolve,
        .open = NULL,
        .read = NULL,
        .seek = NULL,
        .stat = NULL,
        .close = NULL};

struct filesystem *fat32_init()
{
    print("Initialized FAT32\n");
    strcpy(fat32_fs.name, "FAT32");
    return &fat32_fs;
}

struct fat_private
{
    struct fat_h header;
};

int fat32_resolve(struct disk *disk)
{
    int res = 0;
    struct fat_private *fat_private = kzalloc(sizeof(struct fat_private));

    disk->fs_private = fat_private;
    disk->filesystem = &fat32_fs;

    struct disk_stream *stream = diskstreamer_new(disk->id);
    if (!stream)
    {
        res = -ENOMEM;
        goto out;
    }

    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != PEACHOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if (fat_private->header.shared.fat_header_extended_32.BS_BootSig != 0x29)
    {
        res = -EFSNOTUS;
        goto out;
    }

    /*
        if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != PEACHOS_ALL_OK)
        {
            res = -EIO;
            goto out;
        }
        */
    print("Wow, FAT32 resolved!\n");
out:
    if (stream)
    {
        diskstreamer_close(stream);
    }

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

void *fat32_open(struct disk *disk, struct path_part *path, FILE_MODE mode)
{

    if (mode != FILE_MODE_READ)
    {
        return ERROR(-ERDONLY);
    }

    print("Opening file:\n");
    print(path->part);

    return ERROR(-EUNIMP);
}