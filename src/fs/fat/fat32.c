#include "./fat32.h"

#include <stdint.h>
#include <stddef.h>
#include "memory/memory.h"
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
    uint16_t zero_root_dir_entries;
    uint16_t zero_number_of_sectors;
    uint8_t media_type;
    uint16_t zero_sectors_per_fat;
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
int fat32_read(struct disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr);

struct filesystem fat32_fs =
    {
        .resolve = fat32_resolve,
        .open = fat32_open,
        .read = fat32_read,
        .seek = NULL,
        .stat = NULL,
        .close = NULL};

struct filesystem *fat32_init()
{
    strcpy(fat32_fs.name, "FAT32");
    return &fat32_fs;
}

struct fat_private
{
    struct fat_h header;
    struct disk_stream *read_stream;
    /** Buffer to read one cluster */
    uint8_t *buf;
    uint32_t cluster_size_bytes;
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

    fat_private->read_stream = stream;
    fat_private->cluster_size_bytes = fat_private->header.primary_header.bytes_per_sector *
                                      fat_private->header.primary_header.sectors_per_cluster;
    fat_private->buf = kzalloc(fat_private->cluster_size_bytes);

out:

    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_private_file_handle
{
    uint32_t starting_cluster;
    uint32_t file_size;
    uint32_t file_pos;
    uint32_t current_cluster;
    uint32_t position_in_cluster;
};

uint32_t get_cluster_data_disk_pos(struct fat_private *fat_private, uint32_t cluster_id)
{
    return fat_private->header.primary_header.bytes_per_sector *
               fat_private->header.primary_header.reserved_sectors +
           fat_private->header.primary_header.bytes_per_sector *
               fat_private->header.primary_header.fat_copies *
               fat_private->header.shared.fat_header_extended_32.BPB_FATSz32 +
           (cluster_id - 2) *
               fat_private->cluster_size_bytes;
}

uint32_t get_cluster_value(struct fat_private *fat_private, uint32_t cluster_id)
{
    uint32_t value;
    uint32_t cluster_address = fat_private->header.primary_header.bytes_per_sector *
                                   fat_private->header.primary_header.reserved_sectors +
                               cluster_id * sizeof(uint32_t);
    diskstreamer_seek(fat_private->read_stream, cluster_address);
    diskstreamer_read(fat_private->read_stream, &value, sizeof(uint32_t));

    return value;
}

/**
 Return 0 if this cluster is last
 */
uint8_t get_next_cluster_id(struct fat_private *fat_private, uint32_t cluster_id)
{
    uint32_t value = get_cluster_value(fat_private, cluster_id);
    if (value == 0x0FFFFFF8)
    {
        // Last cluster in the chain
        return 0;
    }
    value = value & 0x0FFFFFFF;
    return value;
}

void transform_filename_to_fat(char *dest, const char *src)
{
    uint8_t dest_pos = 0;
    for (dest_pos = 0; dest_pos < 11; dest_pos++)
    {
        dest[dest_pos] = 0x20;
    };
    dest_pos = 0;
    uint8_t src_pos = 0;
    uint8_t is_dot_seen = 0;
    while (dest_pos < 11)
    {
        if (src[src_pos] == 0x0)
        {
            return;
        }
        else if (src[src_pos] == '.')
        {
            if (is_dot_seen)
            {
                // Just ignore this
                src_pos++;
            }
            else
            {
                is_dot_seen = 1;
                src_pos++;
                dest_pos = 8;
            }
        }
        else
        {
            dest[dest_pos] = toupper(src[src_pos]);
            src_pos++;
            dest_pos++;
        }
    }
}

uint32_t get_directory_item_target_cluster_id(struct fat_directory_item *item)
{
    return (item->high_16_bits_first_cluster << 16) +
           item->low_16_bits_first_cluster;
}

uint8_t is_directory_item_directory(struct fat_directory_item *item)
{
    return item->attribute & 0x10;
}

/**
 * Fills target_item with file item in the folder
 * If file/dir is not found then fills with zeros
 */
void get_path_directory_item(struct fat_private *fat_private, struct path_part *path,
                             struct fat_directory_item *target_item)
{
    memset(target_item, 0, sizeof(struct fat_directory_item));

    uint32_t current_cluster_id = fat_private->header.shared.fat_header_extended_32.BPB_RootClus;

    while (1)
    {
        char looking_for_a_file[11];
        transform_filename_to_fat(looking_for_a_file, path->part);
        uint8_t file_found = 0;
        while (1)
        {

            uint32_t current_cluster_data_disk_pos = get_cluster_data_disk_pos(fat_private, current_cluster_id);
            diskstreamer_seek(fat_private->read_stream, current_cluster_data_disk_pos);
            diskstreamer_read(fat_private->read_stream, fat_private->buf, fat_private->cluster_size_bytes);
            uint32_t item_offset = 0;
            while (item_offset < fat_private->cluster_size_bytes)
            {
                struct fat_directory_item *item = (void *)(fat_private->buf + item_offset);

                if (item->filename[0] == 0x0)
                {
                    // No more items in this directory
                    break;
                }

                if (strncmp((char *)item->filename, looking_for_a_file, 11) == 0)
                {
                    file_found = 1;

                    path = path->next;
                    current_cluster_id = get_directory_item_target_cluster_id(item);

                    if (!path)
                    {
                        memcpy(target_item, item, sizeof(struct fat_directory_item));
                        return;
                    }

                    if (path && (!(is_directory_item_directory(item))))
                    {
                        return;
                    }

                    break;
                }

                item_offset += sizeof(struct fat_directory_item);
            }

            if (item_offset < fat_private->cluster_size_bytes)
            {
                // Previous loop exitted earlied, no more files
                break;
            }

            current_cluster_id = get_next_cluster_id(fat_private, current_cluster_id);
            if (!current_cluster_id)
            {
                // This was last cluster
                // Why it happened? It means folder structure is incomplete
                print("Warn: reached last cluster but not the end of directory\n");
                break;
            }
        }
        if (!file_found)
        {
            print("Err: File is not found in this folder\n");
            return;
        }
    }
}

void *fat32_open(struct disk *disk, struct path_part *path, FILE_MODE mode)
{

    if (mode != FILE_MODE_READ)
    {
        return ERROR(-ERDONLY);
    }

    struct fat_private *fat_private = disk->fs_private;

    struct fat_directory_item item;
    get_path_directory_item(fat_private, path, &item);

    if (item.filename[0] == 0)
    {

        return ERROR(-EIO);
    }

    struct fat_private_file_handle *file_handle = kzalloc(sizeof(struct fat_private_file_handle));
    if (!file_handle)
    {
        return ERROR(-ENOMEM);
    }

    file_handle->starting_cluster = get_directory_item_target_cluster_id(&item);
    file_handle->file_size = item.filesize;
    file_handle->current_cluster = file_handle->starting_cluster;
    file_handle->file_pos = 0;
    file_handle->position_in_cluster = 0;

    return file_handle;
}

uint32_t min(uint32_t n1, uint32_t n2)
{
    return n1 < n2 ? n1 : n2;
}

/**
 * Return 1 if all size was readed, overwise zero
 */
int fat32_read_internal(struct fat_private *fat_private, struct fat_private_file_handle *file_handle, uint32_t size, char **out_ptr)
{

    // print("\n[start read] ");
    uint32_t bytes_left_to_read = size;

    while (1)
    {
        // print("\nReading from cluster = ");
        // terminal_writedword(file_handle->current_cluster, 2);
        // print("   ");

        //  [------+++++++]  we are in the middle of cluster
        //                   file might end before cluster ends
        //                   and also we want to read "size" bytes
        uint32_t bytes_available_in_the_cluster = fat_private->cluster_size_bytes -
                                                  file_handle->position_in_cluster;
        uint32_t bytes_in_the_file_till_the_end = file_handle->file_size - file_handle->file_pos;
        uint32_t how_many_to_read_from_this_cluster =
            min(min(bytes_available_in_the_cluster, bytes_in_the_file_till_the_end), bytes_left_to_read);

        // print("clusteravailable=");
        // terminal_writedword(bytes_available_in_the_cluster, 3);
        // print(" filetillend=");
        // terminal_writedword(bytes_in_the_file_till_the_end, 3);
        // print(" lefttoread=");
        // terminal_writedword(bytes_left_to_read, 3);
        // print(" willread=");
        // terminal_writedword(how_many_to_read_from_this_cluster, 3);
        // print(" ");
        //
        // print(" position_in_cluster=");
        // terminal_writedword(file_handle->position_in_cluster, 4);
        // print(" ");

        uint32_t cluster_disk_pos = get_cluster_data_disk_pos(fat_private, file_handle->current_cluster);

        // print(" cluster_disk_pos=");
        // terminal_writedword(cluster_disk_pos, 4);
        // print(" ");

        diskstreamer_seek(fat_private->read_stream, cluster_disk_pos + file_handle->position_in_cluster);
        diskstreamer_read(fat_private->read_stream, *out_ptr, how_many_to_read_from_this_cluster);
        *out_ptr += how_many_to_read_from_this_cluster;
        bytes_left_to_read -= how_many_to_read_from_this_cluster;
        file_handle->file_pos += how_many_to_read_from_this_cluster;
        file_handle->position_in_cluster += how_many_to_read_from_this_cluster;

        if (file_handle->position_in_cluster == fat_private->cluster_size_bytes)
        {
            file_handle->position_in_cluster = 0;
            uint32_t next_cluster_id = get_next_cluster_id(fat_private, file_handle->current_cluster);
            file_handle->current_cluster = next_cluster_id;
        }

        if (!bytes_left_to_read)
        {

            return 1;
        }
        if (file_handle->file_pos == file_handle->file_size)
        {
            return 0;
        }

        if (!file_handle->current_cluster)
        {
            print("ERR: File end is not reached but no more clusters and we still want something to read\n");
            return 0;
        }
    }
}

int fat32_read(struct disk *disk, void *descriptor, uint32_t size, uint32_t nmemb, char *out_ptr)
{
    struct fat_private *fat_private = disk->fs_private;
    struct fat_private_file_handle *file_handle = descriptor;

    int how_many_items_we_readed = 0;
    for (uint32_t i = 0; i < nmemb; i++)
    {
        int this_block_status = fat32_read_internal(fat_private, file_handle, size, &out_ptr);
        how_many_items_we_readed += this_block_status;
        if (this_block_status == 0)
        {
            break;
        }
    }
    return how_many_items_we_readed;
}