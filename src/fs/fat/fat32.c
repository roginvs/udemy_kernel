#include "fat32.h"

#include <stdint.h>
#include <stddef.h>
// #include "memory/memory.h"
#include "string/string.h"

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
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_h
{
    struct fat_header primary_header;
    union fat_h_e
    {
        struct fat_header_extended_32 fat_header_extended_32;
    } shared;
};

struct filesystem fat32_fs =
    {
        .resolve = NULL,
        .open = NULL,
        .read = NULL,
        .seek = NULL,
        .stat = NULL,
        .close = NULL};

struct filesystem *fat32_init()
{
    strcpy(fat32_fs.name, "FAT32");
    return &fat32_fs;
}