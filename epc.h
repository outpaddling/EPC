
/*
 *  Opcodes
 */

// #define OP_J        OP_MOVL     /* j dest = movl dest, pc */
#define OP_JL       0X01
#define OP_RET      OP_MOVL     /* ret = movl (sp)+, pc */

#define OP_BEQ      0X02
#define OP_BNE      0X03

#define OP_BLT      0X04
#define OP_BLE      0X05
#define OP_BLTU     OP_BNC
#define OP_BLEU     0X06

#define OP_BGT      0X07
#define OP_BGE      0X08
#define OP_BGTU     0X09
#define OP_BGEU     OP_BOC

#define OP_BOC      0x0a
#define OP_BNC      0X0B
#define OP_BOV      0X0C
#define OP_BNV      0X0D

#define OP_J        0X0E

#define OP_MOVB     0X10
#define OP_MOVS     0X11
#define OP_MOVL     0X12
#define OP_MOVQ     0X13

#define OP_MOVF     OP_MOVL
#define OP_MOVD     OP_MOVQ

#define OP_MOVLF    0x14
#define OP_MOVFL    0x15
#define OP_MOVFD    0x16
#define OP_MOVDF    0X17

#define OP_CMPB     0X20
#define OP_CMPS     0X21
#define OP_CMPL     0X22
#define OP_CMPQ     0X23
#define OP_CMPF     0X24
#define OP_CMPD     0X25

#define OP_NOTB     0X30
#define OP_NOTS     0X31
#define OP_NOTL     0X32
#define OP_NOTQ     0X33

#define OP_ANDB     0X34
#define OP_ANDS     0X35
#define OP_ANDL     0X36
#define OP_ANDQ     0X37

#define OP_ORB      0X38
#define OP_ORS      0X39
#define OP_ORL      0X3A
#define OP_ORQ      0X3B

#define OP_XORB     0X3C
#define OP_XORS     0X3D
#define OP_XORL     0X3E
#define OP_XORQ     0X3F

#define OP_SLLB     0X40
#define OP_SLLS     0X41
#define OP_SLLL     0X42
#define OP_SLLQ     0X43

#define OP_SRLB     0X44
#define OP_SRLS     0X45
#define OP_SRLL     0X46
#define OP_SRLQ     0X47

#define OP_SRAB     0X48
#define OP_SRAS     0X49
#define OP_SRAL     0X4A
#define OP_SRAQ     0X4B

#define OP_ROLB     0X4C
#define OP_ROLS     0X4D
#define OP_ROLL     0X4E
#define OP_ROLQ     0X4F

#define OP_CLRB     0X50
#define OP_CLRS     0X51
#define OP_CLRL     0X52

#define OP_ADDB     0X60
#define OP_ADDS     0X61
#define OP_ADDL     0X62
#define OP_ADDQ     0X63

#define OP_ADDF     0X64
#define OP_ADDD     0X65

#define OP_ADDBC    0X66
#define OP_ADDSC    0X67
#define OP_ADDLC    0X68

#define OP_INCB     0X69
#define OP_INCS     0X6A
#define OP_INCL     0X6B

#define OP_DECB     0X6C
#define OP_DECS     0X6D
#define OP_DECL     0X6E

#define OP_SUBB     0X70
#define OP_SUBS     0X71
#define OP_SUBL     0X72
#define OP_SUBQ     0X73

#define OP_SUBF     0X74
#define OP_SUBD     0X75

#define OP_MULB     0X80
#define OP_MULS     0X81
#define OP_MULL     0X82
#define OP_MULQ     0X83

#define OP_MULF     0X84
#define OP_MULD     0X85

#define OP_DIVB     0X90
#define OP_DIVS     0X91
#define OP_DIVL     0X92
#define OP_DIVQ     0X93

#define OP_DIVF     0X94
#define OP_DIVD     0X95

#define OP_REMB     0XA0
#define OP_REMS     0XA1
#define OP_REML     0XA2
#define OP_REMQ     0XA3

#define OP_INPUTB   0XB0
#define OP_INPUTS   0XB1
#define OP_INPUTL   0XB2

#define OP_OUTPUTB  0XB4
#define OP_OUTPUTS  0XB5
#define OP_OUTPUTL  0XB6

#define OP_SLEEPL   0xfe
#define OP_HALT     0xff

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

/*
 *  Addressing modes
 */

#define MODE_REG_DIRECT         0x00
#define MODE_REG_INDIRECT       0x10
#define MODE_AUTO_INCREMENT     0x20
#define MODE_AUTO_DECREMENT     0x30
#define MODE_OFFSET             0x40

/*
 *  Modes that don't use registers.  Start with f and use the other nybble
 *  to distinguish.
 */

#define MODE_MEM_DIRECT         0xf0
#define MODE_MEM_INDIRECT       0xe0

/*
 *  Pseudo modes
 */

#define MODE_ADDRESS_OF         MODE_AUTO_INCREMENT
#define MODE_IMMEDIATE          MODE_AUTO_INCREMENT
#define MODE_IMMEDIATE_ADDRESS  MODE_MEM_DIRECT

#define MASK_LOW_BYTE   0x000000ff
#define MASK_LOW_SHORT  0x0000ffff

#define REG_NUM(m)      ((m) & 0x0f)
#define MODE_BITS(m)    ((m) & 0xf0)

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

