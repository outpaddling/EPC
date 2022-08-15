
/*
 *  Opcodes, addressing modes, etc.  Everything that must be consistent
 *  between the assembler and emulator.
 */

#include "ecisc.h"

typedef enum { ECISC, RISC_V } isa_t;

#define EPC_STRING_MAX      1024

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
