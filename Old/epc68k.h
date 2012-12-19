
#define MAGIC_MAX   63

typedef struct
{
    char    magic[MAGIC_MAX+1]; /* #!/usr/local/bin/epc */
    size_t  text_size;
    size_t  data_size;
    size_t  stack_size;
    size_t  heap_size;
}   m68k_bin_header_t;

typedef struct
{
    unsigned long   d0;
    unsigned long   d1;
    unsigned long   d2;
    unsigned long   d3;
    unsigned long   d4;
    unsigned long   d5;
    unsigned long   d6;
    unsigned long   d7;
    unsigned long   a0;
    unsigned long   a1;
    unsigned long   a2;
    unsigned long   a3;
    unsigned long   a4;
    unsigned long   a5;
    unsigned long   a6;
    unsigned long   user_a7;    /* User stack pointer */
    unsigned long   super_a7;   /* Supervisor stack pointer */
    unsigned long   pc;
    unsigned short  status;
}   m68k_reg_t;

#define STATUS_TRACE_MODE       (status & 0x8000)
#define STATUS_SUPERVISOR_MODE  (status & 0x4000)
#define STATUS_INT_MASK         (status & 0x0700)
#define STATUS_EXTEND           (status & 0x0010)
#define STATUS_NEGATIVE         (status & 0x0008)
#define STATUS_ZERO             (status & 0x0004)
#define STATUS_OVERFLOW         (status & 0x0002)
#define STATUS_CARRY            (status & 0x0001)

/*
 *  Stack grows from high to low
 *  push = move.l d0, -(a7)
 *  pop = move (a7)+, d0
 *  Pushing bytes via a7 causes adjustment to word boundary
 *  Not so for a0 - a6
 */

#define SP  a7

/*
 *  Memory-mapped devices
 *  Exception vectors from 0 - 0x3fc
 */

#define KEYBOARD    0x400
#define DISPLAY     0x404

#define VRAM        0x1000  /* 1920 character video RAM area */

#include "protos.h"

