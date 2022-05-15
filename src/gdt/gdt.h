#ifndef GDT_H
#define GDT_H
#include <stdint.h>

// https://wiki.osdev.org/Global_Descriptor_Table
// https://www.geeksforgeeks.org/what-is-global-descriptor-table/

/**
Base address = 32 bit starting memory address of the segment
Segment Limit = 20 bit length of the segment. ( see granularity)

 */
struct gdt
{
    /** Segment limit, bits 0-15 */
    uint16_t segment;
    /** Segment base address, bits 0-15 */
    uint16_t base_first;

    /** Segment base address, bits 16-23 */
    uint8_t base;
    /**

Access Byte
7	6	5	4	3	2	1	0
P	DPL	    S	E	DC	RW	A


P: Present bit. Allows an entry to refer to a valid segment. Must be set (1) for any valid segment.
DPL: Descriptor privilege level field. Contains the CPU Privilege level of the segment. 0 = highest privilege (kernel), 3 = lowest privilege (user applications).
S: Descriptor type bit. If clear (0) the descriptor defines a system segment (eg. a Task State Segment). If set (1) it defines a code or data segment.
E: Executable bit. If clear (0) the descriptor defines a data segment. If set (1) it defines a code segment which can be executed from.
DC: Direction bit/Conforming bit.
For data selectors: Direction bit. If clear (0) the segment grows up. If set (1) the segment grows down, ie. the Offset has to be greater than the Limit.
For code selectors: Conforming bit.
If clear (0) code in this segment can only be executed from the ring set in DPL.
If set (1) code in this segment can be executed from an equal or lower privilege level. For example, code in ring 3 can far-jump to conforming code in a ring 2 segment. The DPL field represent the highest privilege level that is allowed to execute the segment. For example, code in ring 0 cannot far-jump to a conforming code segment where DPL is 2, while code in ring 2 and 3 can. Note that the privilege level remains the same, ie. a far-jump from ring 3 to a segment with a DPL of 2 remains in ring 3 after the jump.
RW: Readable bit/Writable bit.
For code segments: Readable bit. If clear (0), read access for this segment is not allowed. If set (1) read access is allowed. Write access is never allowed for code segments.
For data segments: Writeable bit. If clear (0), write access for this segment is not allowed. If set (1) write access is allowed. Read access is always allowed for data segments.
A: Accessed bit. Best left clear (0), the CPU will set it when the segment is accessed.
     *
     */
    uint8_t access;
    /**
     *  low 4 bits are limit 16:19
     *  high 4 bits are flags

Flags
3	2	1	0
G	DB	L	Reserved

G: Granularity flag, indicates the size the Limit value is scaled by. If clear (0), the Limit is in 1 Byte blocks (byte granularity). If set (1), the Limit is in 4 KiB blocks (page granularity).
DB: Size flag. If clear (0), the descriptor defines a 16-bit protected mode segment. If set (1) it defines a 32-bit protected mode segment. A GDT can have both 16-bit and 32-bit selectors at once.
L: Long-mode code flag. If set (1), the descriptor defines a 64-bit code segment. When set, DB should always be clear. For any other type of segment (other code types or any data segment), it should be clear (0).
     * */
    uint8_t high_flags;
    /** Segment base, bits 24-31 */
    uint8_t base_24_31_bits;
} __attribute__((packed));
;

/**
 This is our user-type to simplify gdt
 */
struct gdt_structured
{
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

void gdt_load(struct gdt *gdt, int size);
void gdt_structured_to_gdt(struct gdt *gdt, struct gdt_structured *structured_gdt, int total_entires);
#endif