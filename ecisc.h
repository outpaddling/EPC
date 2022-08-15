#include <plasm/ecisc-bits.h>

/*
 *  1-byte I/O Addresses
 */

#define ECISC_IO_KEYBOARD     0x00
#define ECISC_IO_VIDEO_MODE   0x01
#define ECISC_IO_SERIAL1      0x02        /* Serial terminal (kb and display) */

/*
 *  Memory map
 */

#define ECISC_MEM_SPACE       0x01000000  /* 16 megabytes */

/*
 *  Cache system caches aligned 32-bit words.
 */

#define ECISC_CACHE_INDEX_BITS    4   /* Cache size = 2 ^ ECISC_CACHE_INDEX_BITS bytes */
#define ECISC_CACHE_SIZE          (1 << ECISC_CACHE_INDEX_BITS)
#define ECISC_CACHE_INDEX_MASK    (0xfffffffful >> (32 - ECISC_CACHE_INDEX_BITS))

/* Extract index, and divide by 4 since there is one entry per 32-bit word */
#define ECISC_CACHE_INDEX(addr)   (((addr) & ECISC_CACHE_INDEX_MASK) >> 2)
#define ECISC_CACHED_ADDRESS(addr) ((addr) >> 2)  /* Only cache aligned addresses */

#define ECISC_IVEC_BASE       0x00000000
#define ECISC_IVEC_END        0x00000400

#define ECISC_FIRMWARE_BASE   0x00001000  /* 1 meg firmware */
#define ECISC_FIRMWARE_END    0x00101000

#define ECISC_USER_BASE       0x00200000
#define ECISC_USER_END        ECISC_MEM_SPACE

#define ECISC_MASK_LOW_BYTE   0x000000ff
#define ECISC_MASK_LOW_SHORT  0x0000ffff


#define ECISC_CPU_REGS        16

#define ECISC_PC  Regs[15]
#define ECISC_SP  Regs[14]

/*
 *  Endian-independent memory access (little-endian regardless of underlying
 *  architecture.
 */

#define ECISC_REG_FLOAT(regnum)   (*(float *)(Regs+(regnum)))

#define ECISC_TRAP_DIV_BY_ZERO    1
#define ECISC_TRAP_BAD_ADDRESS    2

/*
 *  Use small page size so page changes will occur even with small data sets.
 *  This makes it possible to measure locality of reference in a meaningful
 *  way.
 */

#define ECISC_PAGE_SIZE       512

#define ECISC_MEM_CLOCKS      10
#define ECISC_CACHE_CLOCKS    3

