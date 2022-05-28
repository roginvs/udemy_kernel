#include "paging.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
#include "status.h"

void paging_load_directory(uint32_t *directory);

static uint32_t *current_directory = 0;

static void paging_new_4gb_validate(struct paging_4gb_chunk *chunk_4gb)
{
    uint32_t *directory_table = chunk_4gb->directory_entry;

    uint32_t *table_0 = (uint32_t *)(directory_table[0] & 0xFFFFF000);
    if ((table_0[0] & 0xFFFFF000) != 0)
    {
        print("\nERROR: Table 0 entry 0\n");
        return;
    }
    if ((table_0[1] & 0xFFFFF000) != 4096)
    {
        print("\nERROR: Table 0 entry 1\n");
        return;
    }
    if ((table_0[1023] & 0xFFFFF000) != 4096 * 1023)
    {
        print("\nERROR: Table 0 entry 1023\n");
        return;
    }

    uint32_t *table_1 = (uint32_t *)(directory_table[1] & 0xFFFFF000);
    if ((table_1[0] & 0xFFFFF000) != 4096 * 1024 * 1 + 0)
    {
        print("\nERROR: Table 1 entry 0\n");
        return;
    }
    if ((table_1[1] & 0xFFFFF000) != 4096 * 1024 * 1 + 4096)
    {
        print("\nERROR: Table 1 entry 1\n");
        return;
    }
    if ((table_1[1023] & 0xFFFFF000) != 4096 * 1024 * 1 + 4096 * 1023)
    {
        print("\nERROR: Table 1 entry 1023\n");
        return;
    }

    uint32_t *table_1023 = (uint32_t *)(directory_table[1023] & 0xFFFFF000);
    if ((table_1023[0] & 0xFFFFF000) != (uint32_t)(4096) * 1024 * 1023 + 0)
    {
        print("\nERROR: Table 1023 entry 0\n");
        return;
    }
    if ((table_1023[1] & 0xFFFFF000) != (uint32_t)4096 * 1024 * 1023 + 4096)
    {
        print("\nERROR: Table 1023 entry 1\n");
        return;
    }
    if ((table_1023[1023] & 0xFFFFF000) != (uint32_t)4096 * 1024 * 1023 + 4096 * 1023)
    {
        print("\nERROR: Table 1023 entry 1023\n");
        return;
    }
    // print("\npaging_new_4gb is ok\n");
}

struct paging_4gb_chunk *paging_new_4gb_one_chunk(uint8_t flags)
{
    // We will allocate one big chunk of memory:
    // 0 .. 4095: struct paging_4gb_chunk
    // 4096 .. 4096*2-1 : page directory
    // 4096*2 .. 4096*2 + 1024*4096 : entries
    // First table, page
    // 1024*4 = 4096 <- this is first table, page directory
    // 1024 * 1024*4 = 4194304 <- the rest tables, 1024 page tables with 1024 items in each

    uint32_t aligned_size_for_struct = 4096; // Assume this is greater that struct size
    uint32_t table_size = sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE;
    struct paging_4gb_chunk *chunk_4gb = kmalloc(aligned_size_for_struct +
                                                 table_size +
                                                 table_size * PAGING_TOTAL_ENTRIES_PER_TABLE);
    if (((uint32_t)chunk_4gb & 0xFFF) != 0)
    {
        // Alignment failed
        return 0;
    }
    chunk_4gb->directory_entry = (uint32_t *)((char *)chunk_4gb + aligned_size_for_struct);

    char *pages_start = ((char *)chunk_4gb + aligned_size_for_struct + table_size);
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        uint32_t *page_table_location = (uint32_t *)(pages_start + i * table_size);

        chunk_4gb->directory_entry[i] = ((uint32_t)page_table_location) | flags;
        for (int ii = 0; ii < PAGING_TOTAL_ENTRIES_PER_TABLE; ii++)
        {
            page_table_location[ii] = (i * PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE + ii * PAGING_PAGE_SIZE) | flags;
        }
    }

    return chunk_4gb;
}

void paging_free_4gb_one_chunk(struct paging_4gb_chunk *chunk)
{
    kfree(chunk);
}

struct paging_4gb_chunk *paging_new_4gb(uint8_t flags)
{
    uint32_t *directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    // print("Directory address = ");
    // terminal_writedword((uint32_t)directory, 15);
    // print("\n");

    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        uint32_t *entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);

        /*
        if (i == 0)
        {
            print("First page address address = ");
            terminal_writedword((uint32_t)entry, 15);
            print("\n");
        }
        if (i == 1)
        {
            print("Second page address address = ");
            terminal_writedword((uint32_t)entry, 15);
            print("\n");
        }
        */

        for (int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++)
        {
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITEABLE;
    }

    struct paging_4gb_chunk *chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;

    paging_new_4gb_validate(chunk_4gb);
    return chunk_4gb;
}

void paging_switch(struct paging_4gb_chunk *directory)
{
    paging_load_directory(directory->directory_entry);
    current_directory = directory->directory_entry;
}

void paging_free_4gb(struct paging_4gb_chunk *chunk)
{
    for (int i = 0; i < 1024; i++)
    {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t *table = (uint32_t *)(entry & 0xfffff000);
        kfree(table);
    }

    kfree(chunk->directory_entry);
    kfree(chunk);
}

uint32_t *paging_4gb_chunk_get_directory(struct paging_4gb_chunk *chunk)
{
    return chunk->directory_entry;
}

bool paging_is_aligned(void *addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

/**
 * Calculates what directory index and table index will hold the given address
 */
int paging_get_indexes(void *virtual_address, uint32_t *directory_index_out, uint32_t *table_index_out)
{
    int res = 0;
    if (!paging_is_aligned(virtual_address))
    {
        res = -EINVARG;
        goto out;
    }

    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);

out:

    return res;
}

void *paging_align_address(void *ptr)
{
    if ((uint32_t)ptr % PAGING_PAGE_SIZE)
    {
        return (void *)((uint32_t)ptr + PAGING_PAGE_SIZE - ((uint32_t)ptr % PAGING_PAGE_SIZE));
    }

    return ptr;
}
int paging_map(struct paging_4gb_chunk *directory, void *virt, void *phys, int flags)
{
    if (((unsigned int)virt % PAGING_PAGE_SIZE) || ((unsigned int)phys % PAGING_PAGE_SIZE))
    {
        return -EINVARG;
    }

    return paging_set(directory->directory_entry, virt, (uint32_t)phys | flags);
}
int paging_map_range(struct paging_4gb_chunk *directory, void *virt, void *phys, int count, int flags)
{
    int res = 0;
    for (int i = 0; i < count; i++)
    {
        res = paging_map(directory, virt, phys, flags);
        if (res < 0)
            break;
        virt += PAGING_PAGE_SIZE;
        phys += PAGING_PAGE_SIZE;
    }

    return res;
}

int paging_map_to(struct paging_4gb_chunk *directory, void *virt, void *phys, void *phys_end, int flags)
{
    int res = 0;
    if ((uint32_t)virt % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }
    if ((uint32_t)phys % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }
    if ((uint32_t)phys_end % PAGING_PAGE_SIZE)
    {
        res = -EINVARG;
        goto out;
    }

    if ((uint32_t)phys_end < (uint32_t)phys)
    {
        res = -EINVARG;
        goto out;
    }

    uint32_t total_bytes = phys_end - phys;
    int total_pages = total_bytes / PAGING_PAGE_SIZE;
    res = paging_map_range(directory, virt, phys, total_pages, flags);
out:
    return res;
}

/**
 * Updates directory for this virtual address.
 * val  =  physical_address & 0xFFFFF000 + flags;
 */
int paging_set(uint32_t *directory, void *virt, uint32_t val)
{
    if (!paging_is_aligned(virt))
    {
        return -EINVARG;
    }
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    int res = paging_get_indexes(virt, &directory_index, &table_index);
    if (res < 0)
    {
        return res;
    }

    uint32_t entry = directory[directory_index];

    uint32_t *table = (uint32_t *)(
        // Extract address part, no flags needed
        // Page table address is always aligned to 4kb
        entry & 0xFFFFF000);
    table[table_index] = val;

    return 0;
}

uint32_t paging_get(uint32_t *directory, void *virt)
{
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    paging_get_indexes(virt, &directory_index, &table_index);

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xfffff000);
    return table[table_index];
}