
/*
 *  Opcodes, addressing modes, etc.  Everything that must be consistent
 *  between the assembler and emulator.
 */

#include <plasm-epc-bits.h>

/*
 *  1-byte I/O Addresses
 */

#define IO_KEYBOARD     0x00
#define IO_VIDEO_MODE   0x01
#define IO_SERIAL1      0x02        /* Serial terminal (kb and display) */

/*
 *  Memory map
 */

#define MEM_SPACE       0x01000000  /* 16 megabytes */

/*
 *  Cache system caches aligned 32-bit words.
 */

#define CACHE_INDEX_BITS    4   /* Cache size = 2 ^ CACHE_INDEX_BITS bytes */
#define CACHE_SIZE          (1 << CACHE_INDEX_BITS)
#define CACHE_INDEX_MASK    (0xfffffffful >> (32 - CACHE_INDEX_BITS))

/* Extract index, and divide by 4 since there is one entry per 32-bit word */
#define CACHE_INDEX(addr)   (((addr) & CACHE_INDEX_MASK) >> 2)
#define CACHED_ADDRESS(addr) ((addr) >> 2)  /* Only cache aligned addresses */

#define IVEC_BASE       0x00000000
#define IVEC_END        0x00000400

#define FIRMWARE_BASE   0x00001000  /* 1 meg firmware */
#define FIRMWARE_END    0x00101000

#define USER_BASE       0x00200000
#define USER_END        MEM_SPACE

#define STRING_MAX  1024

#define MASK_LOW_BYTE   0x000000ff
#define MASK_LOW_SHORT  0x0000ffff


#define CPU_REGS        16

#define PC  Regs[15]
#define SP  Regs[14]

/*
 *  Endian-independent memory access (little-endian regardless of underlying
 *  architecture.
 */

#define REG_FLOAT(regnum)   (*(float *)(Regs+(regnum)))

#define BYTE_BYTES      1
#define SHORT_BYTES     2
#define LONG_BYTES      4
#define FLOAT_BYTES     4
#define ADDRESS_BYTES   4
#define HEX_BASE        16

#define DEBUG   1

#if DEBUG
#define DPRINTF(...)    { printf(__VA_ARGS__); fflush(stdout); }
#else
#define DPRINTF(...)
#endif

#define TRAP_DIV_BY_ZERO    1
#define TRAP_BAD_ADDRESS    2

/*
 *  Use small page size so page changes will occur even with small data sets.
 *  This makes it possible to measure locality of reference in a meaningful
 *  way.
 */

#define PAGE_SIZE       512

#define STAT_INIT       { 0, 0, 0, 0, 0, 0, 0, 0 }

typedef struct
{
    unsigned long long  instruction_count;
    unsigned long long  clock_cycles;
    unsigned long long  memory_reads;
    unsigned long long  unaligned_reads;
    unsigned long long  memory_writes;
    unsigned long long  unaligned_writes;
    unsigned long long  page_changes;   /* Different page than previous ref */
    unsigned long long  cache_hits;
}   stats_t;

#define MEM_CLOCKS      10
#define CACHE_CLOCKS    3

