#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sysexits.h>
#include <twintk.h>
#include "epc.h"
#include "protos.h"

/*
 *  Global variables should be avoided like the plague, but there are
 *  a precious few situations where their advantage outweighs the problems
 *  they cause.  This is one of them.  Memory and registers are used by
 *  virtually every function in this emulator, so the cost of passing them
 *  as arguments will add up to a major loss in performance.
 *
 *  In typical application programs, I use *NO* global variables.  In
 *  special system software like this, or in embedded applications where
 *  the stack may be limited to 256 bytes, I'll give in and use a few
 *  out of necessity.
 */

uint8_t     *Mem;
uint32_t    *Cache;
uint32_t    Regs[CPU_REGS];
stats_t     Stats = STAT_INIT;

/* Status register */
struct
{
    uint32_t    pad:26;
    uint32_t    d:1;
    uint32_t    i:1;
    uint32_t    v:1;
    uint32_t    n:1;
    uint32_t    c:1;
    uint32_t    z:1;
}   Status = {0,0,0,0,0,0,0};


int     main(int argc,char *argv[])

{
    uint32_t    entry_address;
    
    switch(argc)
    {
	case    2:
	    if ( (entry_address = load_prog(argv[1])) != -1 )
		return run_prog(entry_address);
	    break;
	default:
	    fprintf(stderr,"Usage: %s program\n",argv[0]);
	    return EX_USAGE;
    }
    return EX_OK;
}


inline void mem_read_stats(uint32_t address, uint32_t align_mask)

{
    ++Stats.memory_reads;
    // 1 mem cycle for aligned read, 2 if crossing 32-bit word boundary
    int cache_index = CACHE_INDEX(address);
    
    if ( Cache[cache_index] == CACHED_ADDRESS(address) )
    {
	Stats.clock_cycles += CACHE_CLOCKS;
	++Stats.cache_hits;
    }
    else
    {
	Stats.clock_cycles += MEM_CLOCKS;
	Cache[cache_index] = address;
    }
    if ( (address & align_mask) != 0 )
    {
	Stats.clock_cycles += MEM_CLOCKS;
	++Stats.unaligned_reads;
    }
}


/*
 *  Endian-independent memory access (little-endian regardless of underlying
 *  architecture.
 */

inline uint32_t read_mem_long(uint32_t address)

{
    mem_read_stats(address, 0x3);
    return Mem[address] \
	| (Mem[address+1] << 8)
	| (Mem[address+2] << 16)
	| (Mem[address+3] << 24);
}


inline uint16_t read_mem_short(uint32_t address)

{
    mem_read_stats(address, 0x3);
    return Mem[address]
	| (Mem[address+1] << 8);
}


inline uint8_t  read_mem_byte(uint32_t address)

{
    mem_read_stats(address, 0x0);
    return Mem[address];
}


inline void mem_write_stats(uint32_t address, uint32_t align_mask)

{
    //printf("Writing %lu\n", address);
    ++Stats.memory_writes;
    // 1 mem cycle for aligned read, 2 if crossing 32-bit word boundary
    Stats.clock_cycles += MEM_CLOCKS;
    if ( address & align_mask )
    {
	Stats.clock_cycles += MEM_CLOCKS;
	++Stats.unaligned_writes;
    }
}


void    write_mem_long(uint32_t address, uint32_t val)

{
    mem_write_stats(address, 0x3);
    Mem[address] = (val) & 0xff,
    Mem[address+1] = ((val) & 0xff00) >> 8,
    Mem[address+2] = ((val) & 0xff0000) >> 16,
    Mem[address+3] = ((val) & 0xff000000) >> 24;
}


void    write_mem_short(uint32_t address, uint16_t val)

{
    mem_write_stats(address, 0x3);
    Mem[address] = (val) & 0xff,
    Mem[address+1] = ((val) & 0xff00) >> 8;
}


inline void write_mem_byte(uint32_t address, uint8_t val)

{
    mem_write_stats(address, 0x0);
    
    Mem[address] = val;
}


inline float    fetch_float(uint32_t address)

{
    uint32_t    num;
    
    num = read_mem_long(address);
    
    return *(float *)&num;
}


void    store_float(uint32_t address, float num)

{
    write_mem_long(address, *(uint32_t *)&num);
}


inline uint8_t fetch_byte_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    return Regs[REG_NUM(mode_byte)] & MASK_LOW_BYTE;
	    
	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    return read_mem_byte(address);  // Mem[address];
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)]++;
	    return read_mem_byte(address);  // Mem[address];
	    
	case    MODE_AUTO_DECREMENT:
	    address = --Regs[REG_NUM(mode_byte)];
	    return read_mem_byte(address);  // Mem[address];
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    /* DPRINTF("offset = %08x  r%d = %08x\n",
		offset, REG_NUM(mode_byte), Regs[REG_NUM(mode_byte)]); */
	    return read_mem_byte(offset + Regs[REG_NUM(mode_byte)]);
	    // Mem[offset + Regs[REG_NUM(mode_byte)]];
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return read_mem_byte(address);  // Mem[address];
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    //DPRINTF("pointer = %08x\n", pointer);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    //DPRINTF("address = %08x\n", address);
	    return read_mem_byte(address);  // Mem[address];

	default:
	    fprintf(stderr, "fetch_byte_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


inline uint16_t fetch_short_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    return Regs[REG_NUM(mode_byte)] & MASK_LOW_SHORT;
	    
	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    return read_mem_short(address);
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)];
	    Regs[REG_NUM(mode_byte)] += SHORT_BYTES;
	    //DPRINTF("fetch_short: PC = %08x\n", PC);
	    return read_mem_short(address);
	    
	case    MODE_AUTO_DECREMENT:
	    Regs[REG_NUM(mode_byte)] -= SHORT_BYTES;
	    address = Regs[REG_NUM(mode_byte)];
	    return read_mem_short(address);
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return read_mem_short(offset + Regs[REG_NUM(mode_byte)]);
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return read_mem_short(address);
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return read_mem_short(address);

	default:
	    fprintf(stderr, "fetch_short_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


inline uint32_t get_effective_address(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_INDIRECT:
	    return Regs[REG_NUM(mode_byte)];
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)];
	    Regs[REG_NUM(mode_byte)] += LONG_BYTES;
	    return address;
	    
	case    MODE_AUTO_DECREMENT:
	    Regs[REG_NUM(mode_byte)] -= LONG_BYTES;
	    address = Regs[REG_NUM(mode_byte)];
	    return address;
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return offset + Regs[REG_NUM(mode_byte)];
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return address;
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return address;

	default:
	    fprintf(stderr, "get_effective_address: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


inline uint32_t fetch_long_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    return Regs[REG_NUM(mode_byte)];
	    
	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    return read_mem_long(address);
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)];
	    Regs[REG_NUM(mode_byte)] += LONG_BYTES;
	    return read_mem_long(address);
	    
	case    MODE_AUTO_DECREMENT:
	    Regs[REG_NUM(mode_byte)] -= LONG_BYTES;
	    address = Regs[REG_NUM(mode_byte)];
	    return read_mem_long(address);
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return read_mem_long(offset + Regs[REG_NUM(mode_byte)]);
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return read_mem_long(address);
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return read_mem_long(address);

	default:
	    fprintf(stderr, "fetch_long_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


inline float fetch_float_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    return REG_FLOAT(REG_NUM(mode_byte));
	    
	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    return fetch_float(address);
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)];
	    Regs[REG_NUM(mode_byte)] += FLOAT_BYTES;
	    return fetch_float(address);
	    
	case    MODE_AUTO_DECREMENT:
	    Regs[REG_NUM(mode_byte)] -= FLOAT_BYTES;
	    address = Regs[REG_NUM(mode_byte)];
	    return fetch_float(address);
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return fetch_float(offset + Regs[REG_NUM(mode_byte)]);
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    return fetch_float(address);
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return fetch_float(address);

	default:
	    fprintf(stderr, "fetch_float_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


inline void store_byte_operand(uint8_t operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    Regs[REG_NUM(mode_byte)] = 
		(Regs[REG_NUM(mode_byte)] & 0xffffff00) | operand;
	    return;

	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)]++;
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	    
	case    MODE_AUTO_DECREMENT:
	    address = --Regs[REG_NUM(mode_byte)];
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    write_mem_byte(offset + Regs[REG_NUM(mode_byte)], operand);
	    // Mem[offset + Regs[REG_NUM(mode_byte)]] = operand;
	    return;
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;

	default:
	    fprintf(stderr, "store_byte_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


inline void store_short_operand(uint16_t operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    Regs[REG_NUM(mode_byte)] = 
		(Regs[REG_NUM(mode_byte)] & 0xffff0000) | operand;
	    return;

	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    write_mem_short(address, operand);
	    return;
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)];
	    Regs[REG_NUM(mode_byte)] += SHORT_BYTES;
	    write_mem_short(address, operand);
	    return;
	    
	case    MODE_AUTO_DECREMENT:
	    Regs[REG_NUM(mode_byte)] -= SHORT_BYTES;
	    address = Regs[REG_NUM(mode_byte)];
	    write_mem_short(address, operand);
	    return;
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    write_mem_short(address, operand);
	    return;
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    write_mem_short(address, operand);
	    return;
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    write_mem_short(address, operand);
	    return;

	default:
	    fprintf(stderr, "store_short_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


inline void store_long_operand(uint32_t operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    //DPRINTF("Storing %08x\n", operand);
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    Regs[REG_NUM(mode_byte)] = operand;
	    return;

	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    write_mem_long(address, operand);
	    return;
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)];
	    Regs[REG_NUM(mode_byte)] += LONG_BYTES;
	    write_mem_long(address, operand);
	    return;
	    
	case    MODE_AUTO_DECREMENT:
	    Regs[REG_NUM(mode_byte)] -= LONG_BYTES;
	    address = Regs[REG_NUM(mode_byte)];
	    write_mem_long(address, operand);
	    return;
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    write_mem_long(address, operand);
	    return;
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    write_mem_long(address, operand);
	    //DPRINTF("Storing %08x at %08x\n", operand, address);
	    return;
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    write_mem_long(address, operand);
	    return;

	default:
	    fprintf(stderr, "store_long_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


inline void store_float_operand(float operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    //DPRINTF("Storing %f\n", operand);
    mode_byte = read_mem_byte(PC++);    // Mem[PC++];
    switch(MODE_BITS(mode_byte))
    {
	case    MODE_REG_DIRECT:
	    REG_FLOAT(REG_NUM(mode_byte)) = operand;
	    return;

	case    MODE_REG_INDIRECT:
	    address = Regs[REG_NUM(mode_byte)];
	    store_float(address, operand);
	    return;
	    
	case    MODE_AUTO_INCREMENT:
	    address = Regs[REG_NUM(mode_byte)];
	    Regs[REG_NUM(mode_byte)] += FLOAT_BYTES;
	    store_float(address, operand);
	    return;
	    
	case    MODE_AUTO_DECREMENT:
	    Regs[REG_NUM(mode_byte)] -= FLOAT_BYTES;
	    address = Regs[REG_NUM(mode_byte)];
	    store_float(address, operand);
	    return;
	    
	case    MODE_OFFSET:
	    offset = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    store_float(address, operand);
	    return;
	
	case    MODE_MEM_DIRECT:
	    address = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    store_float(address, operand);
	    //DPRINTF("Storing %08x at %08x\n", operand, address);
	    return;
	
	case    MODE_MEM_INDIRECT:
	    pointer = read_mem_long(PC);
	    PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    store_float(address, operand);
	    return;

	default:
	    fprintf(stderr, "store_float_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


inline void jl(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    /* Push PC */
    SP -= ADDRESS_BYTES;
    write_mem_long(SP, PC);
    
    /* Branch */
    PC = address;
}


inline void jeq(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.z == 1 )
	PC = address;
}


inline void jne(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.z == 0 )
	PC = address;
}


inline void jlt(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.n == 1 )
	PC = address;
}


inline void jle(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.n == 1) || (Status.z == 1) )
	PC = address;
}


inline void jltu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.c == 0 )
	PC = address;
}


inline void jleu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.c == 0) || (Status.z == 1) )
	PC = address;
}


inline void jgt(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.n == 0) && (Status.z == 0) )
	PC = address;
}


inline void jge(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.n == 0) || (Status.z == 1) )
	PC = address;
}


inline void jgtu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.c == 1) && (Status.z == 0) )
	PC = address;
}


inline void jgeu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.c == 1 )
	PC = address;
}


inline void joc(void)

{
    jgeu();
}


inline void jnc(void)

{
    jltu();
}


inline void jov(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.v == 1 )
	PC = address;
}


inline void jnv(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.v == 0 )
	PC = address;
}


inline void j(void)

{
    PC = get_effective_address();
}


inline void movb(void)

{
    store_byte_operand(fetch_byte_operand());
}


inline void movs(void)

{
    store_short_operand(fetch_short_operand());
}


inline void movl(void)

{
    store_long_operand(fetch_long_operand());
}


inline void movq(void)

{
    fprintf(stderr, "movq not yet implemented.\n");
}


inline void     movfl(void)

{
    store_long_operand((long)fetch_float_operand());
}


inline void     movlf(void)

{
    store_float_operand((float)fetch_long_operand());
}


inline void     movfd(void)

{
    fprintf(stderr, "movdf not yet implemented.\n");
}


inline void     movdf(void)

{
    fprintf(stderr, "movdf not yet implemented.\n");
}


inline void cmpb(void)

{
    static int8_t   diff;
    static uint8_t  operand1, operand2;
    
    operand1 = fetch_byte_operand();
    operand2 = fetch_byte_operand();
    diff = operand1 - operand2;
    
    //fprintf(stderr, "cmpb: %d %d diff = %d\n", Operand1, operand2, diff);
    Status.z = (diff == 0);
    Status.n = (diff < 0);
    Status.c = (operand1 >= operand2);
    /* Overfow is unaffected by cmp instructions */
}


inline void     cmps(void)

{
    static int16_t   diff;
    static uint16_t  operand1, operand2;
    
    operand1 = fetch_short_operand();
    operand2 = fetch_short_operand();
    diff = operand1 - operand2;
    
    //fprintf(stderr, "cmpb: %d %d diff = %d\n", Operand1, operand2, diff);
    Status.z = (diff == 0);
    Status.n = (diff < 0);
    Status.c = (operand1 >= operand2);
    /* Overfow is unaffected by cmp instructions */
}


inline void     cmpl(void)

{
    static int32_t   diff;
    static uint32_t  operand1, operand2;
    
    operand1 = fetch_long_operand();
    operand2 = fetch_long_operand();
    diff = operand1 - operand2;
    
    //fprintf(stderr, "cmpb: %d %d diff = %d\n", Operand1, operand2, diff);
    Status.z = (diff == 0);
    Status.n = (diff < 0);
    Status.c = (operand1 >= operand2);
    /* Overfow is unaffected by cmp instructions */
}


inline void     cmpq(void)

{
    fprintf(stderr, "cmpq not yet implemented.\n");
}


inline void     cmpf(void)

{
    static float    diff, operand1, operand2;
    
    operand1 = fetch_float_operand();
    operand2 = fetch_float_operand();
    diff = operand1 - operand2;
    
    //DPRINTF("cmpf: %f %f diff = %f\n", operand1, operand2, diff);
    Status.z = (diff == 0);
    Status.n = (diff < 0);
    /* Carry unaffected by floating point compare */
    /* Overfow is unaffected by cmp instructions */
}


inline void     cmpd(void)

{
    fprintf(stderr, "cmpd not yet implemented.\n");
}


inline void     notb(void)

{
    store_byte_operand(~fetch_byte_operand());
}


inline void     nots(void)

{
    store_short_operand(~fetch_short_operand());
}


inline void     notl(void)

{
    store_long_operand(~fetch_long_operand());
}


inline void     notq(void)

{
    fprintf(stderr, "notq not yet implemented.\n");
}


inline void     andb(void)

{
    store_byte_operand(fetch_byte_operand() & fetch_byte_operand());
}


inline void     ands(void)

{
    store_short_operand(fetch_short_operand() & fetch_short_operand());
}


inline void     andl(void)

{
    store_long_operand(fetch_long_operand() & fetch_long_operand());
}


inline void     andq(void)

{
    fprintf(stderr, "andq not yet implemented.\n");
}


inline void     orb(void)

{
    store_byte_operand(fetch_byte_operand() | fetch_byte_operand());
}


inline void     ors(void)

{
    store_short_operand(fetch_short_operand() & fetch_short_operand());
}


inline void     orl(void)

{
    store_long_operand(fetch_long_operand() & fetch_long_operand());
}


inline void     orq(void)

{
    fprintf(stderr, "orq not yet implemented.\n");
}


inline void     xorb(void)

{
    store_byte_operand(fetch_byte_operand() ^ fetch_byte_operand());
}


inline void     xors(void)

{
    store_short_operand(fetch_short_operand() ^ fetch_short_operand());
}


inline void     xorl(void)

{
    store_long_operand(fetch_long_operand() ^ fetch_long_operand());
}


inline void     xorq(void)

{
    fprintf(stderr, "xorq not yet implemented.\n");
}


inline void     sllb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint8_t operand1 = fetch_byte_operand();  /* Byte to shift */
    
    /* Second operand is shift count */
    store_byte_operand(operand1 << fetch_byte_operand());
}


inline void     slls(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint16_t operand1 = fetch_short_operand();  /* Short to shift */
    
    /* Second operand is shift count */
    store_short_operand(operand1 << fetch_short_operand());
}


inline void     slll(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint32_t operand1 = fetch_long_operand();  /* Long to shift */
    
    /* Second operand is shift count */
    store_long_operand(operand1 << fetch_long_operand());
}


inline void     sllq(void)

{
    fprintf(stderr, "sllq not yet implemented.\n");
}


inline void     srlb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint8_t operand1 = fetch_byte_operand();  /* Byte to shift */
    
    /* Second operand is shift count */
    store_byte_operand(operand1 >> fetch_byte_operand());
}


inline void     srls(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint16_t operand1 = fetch_short_operand();  /* Short to shift */
    
    /* Second operand is shift count */
    store_short_operand(operand1 >> fetch_short_operand());
}


inline void     srll(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
   
    uint32_t operand1 = fetch_long_operand();  /* Long to shift */
    
    /* Second operand is shift count */
    store_long_operand(operand1 >> fetch_long_operand());
}


inline void     srlq(void)

{
    fprintf(stderr, "srlq not yet implemented.\n");
}


inline void     srab(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t operand1 = fetch_byte_operand();  /* Byte to shift */
    
    /* Second operand is shift count */
    store_byte_operand(operand1 >> fetch_byte_operand());
}


inline void     sras(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t operand1 = fetch_short_operand();  /* Short to shift */
    
    /* Second operand is shift count */
    store_short_operand(operand1 >> fetch_short_operand());
}


inline void     sral(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int32_t operand1 = fetch_long_operand();  /* Long to shift */
    
    /* Second operand is shift count */
    store_long_operand(operand1 >> fetch_long_operand());
}


inline void     sraq(void)

{
    fprintf(stderr, "sraq not yet implemented.\n");
}


inline void     rolb(void)

{
    fprintf(stderr, "rolb not yet implemented.\n");
}


inline void     rols(void)

{
    fprintf(stderr, "rols not yet implemented.\n");
}


inline void     roll(void)

{
    fprintf(stderr, "roll not yet implemented.\n");
}


inline void     rolq(void)

{
    fprintf(stderr, "rolq not yet implemented.\n");
}


inline void     addb(void)

{
    store_byte_operand(fetch_byte_operand() + fetch_byte_operand());
}


inline void     adds(void)

{
    store_short_operand(fetch_short_operand() + fetch_short_operand());
}


inline void     addl(void)

{
    store_long_operand(fetch_long_operand() + fetch_long_operand());
}


inline void     addq(void)

{
    fprintf(stderr, "addq not yet implemented.\n");
}


inline void     addf(void)

{
    static float    operand1, operand2;
    
    operand1 = fetch_float_operand();
    operand2 = fetch_float_operand();
    //DPRINTF("addf: %f + %f = %f\n", operand1, operand2, operand1 + operand2);
    store_float_operand(operand1 + operand2);
}


inline void     addd(void)

{
    fprintf(stderr, "addd not yet implemented.\n");
}


inline void subb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t operand1 = fetch_byte_operand();
    
    /* Second operand is shift count */
    store_byte_operand(operand1 - fetch_byte_operand());
}


inline void subs(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t operand1 = fetch_short_operand();
    
    /* Second operand is shift count */
    store_short_operand(operand1 - fetch_short_operand());
}


inline void subl(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int32_t operand1 = fetch_long_operand();
    int32_t operand2 = fetch_long_operand();
    
    /* Second operand is shift count */
    store_long_operand(operand1 - operand2);
}


inline void     subq(void)

{
    fprintf(stderr, "subq not yet implemented.\n");
}


inline void     subf(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    float   operand1 = fetch_float_operand();
    float   operand2 = fetch_float_operand();
    
    /* Second operand is shift count */
    store_float_operand(operand1 - operand2);
}


inline void     subd(void)

{
    fprintf(stderr, "subd not yet implemented.\n");
}


inline void     mulb(void)

{
    store_byte_operand(fetch_byte_operand() * fetch_byte_operand());
}


inline void     muls(void)

{
    store_short_operand(fetch_short_operand() * fetch_short_operand());
}


inline void     mull(void)

{
    store_long_operand(fetch_long_operand() * fetch_long_operand());
}


inline void     mulq(void)

{
    fprintf(stderr, "mulq not yet implemented.\n");
}


inline void     mulf(void)

{
    store_float_operand(fetch_float_operand() * fetch_float_operand());
}


inline void     muld(void)

{
    fprintf(stderr, "muld not yet implemented.\n");
}


inline void     divb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t dividend = fetch_byte_operand();
    int8_t divisor = fetch_byte_operand();
    
    if ( divisor == 0 )
	trap(TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_byte_operand(dividend / divisor);
}


inline void     divs(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t dividend = fetch_short_operand();
    int16_t divisor = fetch_short_operand();
    
    if ( divisor == 0 )
	trap(TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_short_operand(dividend / divisor);
}


inline void     divl(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int32_t dividend = fetch_long_operand();
    int32_t divisor = fetch_long_operand();
    
    if ( divisor == 0 )
	trap(TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_long_operand(dividend / divisor);
}


inline void     divq(void)

{
    fprintf(stderr, "divq not yet implemented.\n");
}


inline void     divf(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    float   dividend = fetch_float_operand();
    float   divisor = fetch_float_operand();
    
    if ( divisor == 0.0 )
	trap(TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_float_operand(dividend / divisor);
}


inline void     divd(void)

{
    fprintf(stderr, "divd not yet implemented.\n");
}


inline void     remb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t dividend = fetch_byte_operand();
    int8_t divisor = fetch_byte_operand();
    
    if ( divisor == 0 )
	trap(TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_byte_operand(dividend % divisor);
}


inline void     rems(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t dividend = fetch_short_operand();
    int16_t divisor = fetch_short_operand();
    
    if ( divisor == 0 )
	trap(TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_short_operand(dividend % divisor);
}


inline void     reml(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int32_t dividend = fetch_long_operand();
    int32_t divisor = fetch_long_operand();
    
    if ( divisor == 0 )
	trap(TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_long_operand(dividend % divisor);
}


inline void     remq(void)

{
    fprintf(stderr, "remq not yet implemented.\n");
}


inline void     inputb(void)

{
    /* First operand is I/O address */
    switch(fetch_byte_operand())
    {
	case    IO_SERIAL1:
	    store_byte_operand(getchar());
	    break;
	default:
	    break;
    }
}


inline void     inputs(void)

{
    fprintf(stderr, "inputs not yet implemented.\n");
}


inline void     inputl(void)

{
    fprintf(stderr, "inputl not yet implemented.\n");
}


inline void outputb(void)

{
    static uint8_t operand1;
    
    operand1 = fetch_byte_operand();    /* Byte to output */
    
    /* Second operand is I/O address */
    switch(fetch_byte_operand())
    {
	case    IO_SERIAL1:
	    putchar(operand1);
	    fflush(stdout);
	    break;
	default:
	    break;
    }
}


inline void sleepl(void)

{
    usleep(fetch_long_operand());
}


inline void     outputs(void)

{
    fprintf(stderr, "outputs not yet implemented.\n");
}


inline void     outputl(void)

{
    fprintf(stderr, "outputl not yet implemented.\n");
}


/***************************************************************************
 * Description:
 *
 * Arguments:
 *
 * Returns:
 *
 * History: 
 *
 ***************************************************************************/

int     run_prog(uint32_t entry_address)

{
    uint32_t    opcode;
    term_t      *terminal;
    
    /* Initialize stats */
    init_stats();
    
    /* Set up terminal */
    terminal = init_term(stdin, stdout, stderr, NULL, 0);
    tpush_tty(terminal);
    tcbreak_mode(terminal);
    
    /* Initialize Program_counter */
    // PC = USER_BASE;
    PC = USER_BASE + entry_address;
    DPRINTF("Starting program at 0x%08X\r\n", PC);

    /* Initialize stack */
    SP = USER_END;
    
    do
    {
	//DPRINTF("address: %08x ", PC);
	opcode = read_mem_byte(PC++);   // Mem[PC++];
	//DPRINTF("Opcode: %02x\r\n", opcode);
	
	switch( opcode )
	{
	    case    OP_JL:
		jl();
		break;

	    case    OP_BEQ:
		jeq();
		break;

	    case    OP_BNE:
		jne();
		break;

	    case    OP_BLT:
		jlt();
		break;

	    case    OP_BLE:
		jle();
		break;

	    case    OP_BLTU:    /* Same as jnc */
		jltu();
		break;

	    case    OP_BLEU:
		jleu();
		break;

	    case    OP_BGT:
		jgt();
		break;

	    case    OP_BGE:
		jge();
		break;

	    case    OP_BGTU:
		jgtu();
		break;

	    case    OP_BGEU:    /* Same as joc */
		jgeu();
		break;

	    case    OP_BOV:
		jov();
		break;

	    case    OP_BNV:
		jnv();
		break;

	    case    OP_J:
		j();
		break;

	    case    OP_MOVB:
		movb();
		break;

	    case    OP_MOVS:
		movs();
		break;

	    case    OP_MOVL:    /* Also movf */
		movl();
		break;

	    case    OP_MOVQ:    /* Also movd */
		movq();
		break;

	    case    OP_MOVFL:
		movfl();
		break;

	    case    OP_MOVLF:
		movlf();
		break;

	    case    OP_MOVFD:
		movfd();
		break;

	    case    OP_MOVDF:
		movdf();
		break;

	    case    OP_CMPB:
		cmpb();
		break;

	    case    OP_CMPS:
		cmps();
		break;

	    case    OP_CMPL:
		cmpl();
		break;

	    case    OP_CMPQ:
		cmpq();
		break;

	    case    OP_CMPF:
		cmpf();
		break;

	    case    OP_CMPD:
		cmpd();
		break;

	    case    OP_NOTB:
		notb();
		break;

	    case    OP_NOTS:
		nots();
		break;

	    case    OP_NOTL:
		notl();
		break;

	    case    OP_NOTQ:
		notq();
		break;

	    case    OP_ANDB:
		andb();
		break;

	    case    OP_ANDS:
		ands();
		break;

	    case    OP_ANDL:
		andl();
		break;

	    case    OP_ANDQ:
		andq();
		break;

	    case    OP_ORB:
		orb();
		break;

	    case    OP_ORS:
		ors();
		break;

	    case    OP_ORL:
		orl();
		break;

	    case    OP_ORQ:
		orq();
		break;

	    case    OP_XORB:
		xorb();
		break;

	    case    OP_XORS:
		xors();
		break;

	    case    OP_XORL:
		xorl();
		break;

	    case    OP_XORQ:
		xorq();
		break;

	    case    OP_SLLB:
		sllb();
		break;

	    case    OP_SLLS:
		slls();
		break;

	    case    OP_SLLL:
		slll();
		break;

	    case    OP_SLLQ:
		sllq();
		break;

	    case    OP_SRLB:
		srlb();
		break;

	    case    OP_SRLS:
		srls();
		break;

	    case    OP_SRLL:
		srll();
		break;

	    case    OP_SRLQ:
		srlq();
		break;

	    case    OP_SRAB:
		srab();
		break;

	    case    OP_SRAS:
		sras();
		break;

	    case    OP_SRAL:
		sral();
		break;

	    case    OP_SRAQ:
		sraq();
		break;

	    case    OP_ROLB:
		rolb();
		break;

	    case    OP_ROLS:
		rols();
		break;

	    case    OP_ROLL:
		roll();
		break;

	    case    OP_ROLQ:
		rolq();
		break;

	    case    OP_ADDB:
		addb();
		break;

	    case    OP_ADDS:
		adds();
		break;

	    case    OP_ADDL:
		addl();
		break;

	    case    OP_ADDQ:
		addq();
		break;

	    case    OP_ADDF:
		addf();
		break;

	    case    OP_ADDD:
		addd();
		break;

	    case    OP_SUBB:
		subb();
		break;

	    case    OP_SUBS:
		subs();
		break;

	    case    OP_SUBL:
		subl();
		break;

	    case    OP_SUBQ:
		subq();
		break;

	    case    OP_SUBF:
		subf();
		break;

	    case    OP_SUBD:
		subd();
		break;

	    case    OP_MULB:
		mulb();
		break;

	    case    OP_MULS:
		muls();
		break;

	    case    OP_MULL:
		mull();
		break;

	    case    OP_MULQ:
		mulq();
		break;

	    case    OP_MULF:
		mulf();
		break;

	    case    OP_MULD:
		muld();
		break;

	    case    OP_DIVB:
		divb();
		break;

	    case    OP_DIVS:
		divs();
		break;

	    case    OP_DIVL:
		divl();
		break;

	    case    OP_DIVQ:
		divq();
		break;

	    case    OP_DIVF:
		divf();
		break;

	    case    OP_DIVD:
		divd();
		break;

	    case    OP_REMB:
		remb();
		break;

	    case    OP_REMS:
		rems();
		break;

	    case    OP_REML:
		reml();
		break;

	    case    OP_REMQ:
		remq();
		break;

	    case    OP_INPUTB:
		inputb();
		break;

	    case    OP_INPUTS:
		inputs();
		break;

	    case    OP_INPUTL:
		inputl();
		break;

	    case    OP_OUTPUTB:
		outputb();
		break;

	    case    OP_OUTPUTS:
		outputs();
		break;

	    case    OP_OUTPUTL:
		outputl();
		break;

	    case    OP_SLEEPL:
		sleepl();
		break;
		
	    case    OP_HALT:
		break;

	    default:
		illegal_instruction(opcode);
	}
	
	/* Count instruction */
	++Stats.instruction_count;
	
	/*
	 *  Assume one clock for each execute phase.  Some instructions,
	 *  such as mul and div may add more.
	 */
	++Stats.clock_cycles;
	
    }   while ( opcode != OP_HALT ); //opcode != OP_HALT );

    tpop_tty(terminal);
    
    print_stats(&Stats);
    
    return EX_OK;
}


void    print_stats(stats_t *stats)

{
    printf("%qu instructions executed.\n", stats->instruction_count);
    // Clock cycles
    /* Not finished */
    printf("%qu clock cycles (ball park).\n", stats->clock_cycles);
    // Memory references
    printf("%qu memory reads, %qu unaligned.\n",
	    stats->memory_reads, stats->unaligned_reads);
    /* Not finished printf("%qu cache hits, hit ratio = %f.\n",
	    stats->cache_hits, (double)stats->cache_hits / stats->memory_reads); */
    printf("%qu memory writes, %qu unaligned.\n",
	    stats->memory_writes, stats->unaligned_writes);
    // Locality of reference
}


int     read_string(FILE *fp, char *string, size_t maxlen)

{
    int     c, ch;
    
    while ( isspace(ch = getc(fp)) )
	;
	
    for (c=0; (ch != EOF) && (c < maxlen) && !isspace(ch); ++c)
    {
	string[c] = ch;
	ch = getc(fp);
    }
    string[c] = '\0';
    return c;
}


uint32_t    load_prog(char *filename)

{
    size_t      offset;
    uint32_t    address,
		entry_address = -1;
    FILE        *fp;
    char        string[STRING_MAX+1], name[STRING_MAX+1], *end;
    size_t      len;
    uint32_t    val;
    
    Mem = malloc(MEM_SPACE);
    Cache = malloc(CACHE_SIZE * 1); /* One entry per 32-bit word */
    
    fp = fopen(filename, "r");
    if ( fp == NULL )
    {
	fprintf(stderr, "%s: Cannot open %s: %s\n",
	    __func__, filename, strerror(errno));
	exit(EX_DATAERR);
    }

    /* Read header */
    /*
    read_string(fp, string, STRING_MAX);
    if ( strcasecmp(string, "!EPC00") != 0 )
    {
	fputs("This does not appear to be an EPC executable.\n", stderr);
	fputs("Expected !EPC00.\n", stderr);
	exit(EX_DATAERR);
    }
    */
    
    /* Read text segment */
    offset = USER_BASE;
    do
    {
	/* Read and discard offset */
	read_string(fp, string, STRING_MAX);
	
	/* End of program marked by End tag */
	if ( strcasecmp(string, "end") == 0 )
	    break;
	
	/* Read machine code */
	do
	{
	    len = read_string(fp, string, STRING_MAX);
	    
	    /* Read and discard comment */
	    if ( *string == '#' )
	    {
		while ( getc(fp) != '\n' )
		    ;
		break;
	    }
	    else
	    {
		//fprintf(stderr, "%lu '%s'\n", (unsigned long)len, string);
		char    *p = string;
		
		/* Addresses are tagged with '@'.  Add base address. */
		if ( *p != '@' )
		    val = strtoull(p, &end, HEX_BASE);
		else
		{
		    val = USER_BASE + strtoull(++p, &end, HEX_BASE);
		    --len;
		}
		if ( end != p + len )
		{
		    fprintf(stderr, "Invalid hex string: %s\n", p);
		    exit(EX_DATAERR);
		}
		switch(len)
		{
		    case    2:
			write_mem_byte(offset, val);    // Mem[offset] = val;
			break;
		    case    4:
			*(uint16_t *)(Mem+offset) = val;
			break;
		    case    8:
			write_mem_long(offset, val);
			break;
		    case    16:
			*(uint64_t *)(Mem+offset) = val;
			break;
		    default:
			fprintf(stderr, "Invalid hex value: %s\n", string);
			exit(EX_DATAERR);
		}
		offset += len / 2;  /* Add # bytes */
	    }
	}   while ( *string != '#' );
    }   while ( len > 0 );
    
    /* Read symbol table */
    while ( fgets(string, STRING_MAX, fp) != NULL )
    {
	if ( sscanf(string, "%X %s", &address, name) == 2 )
	{
	    if ( strcmp(name, "main") == 0 )
	    {
		entry_address = address;
		//DPRINTF("Offset of main = %08X\r\n", entry_address);
	    }
	}
    }
    
    fclose(fp);
    return entry_address;
}


void    illegal_instruction(uint8_t opcode)

{
    fprintf(stderr, "Illegal instruction: %02x\n", opcode);
    exit(EX_DATAERR);
}


void    illegal_addressing_mode(uint8_t mode_byte)

{
    fprintf(stderr,"Illegal mode byte: %02x\n", mode_byte);
    exit(EX_DATAERR);
}


void    trap(int code)

{
    switch(code)
    {
	case    TRAP_DIV_BY_ZERO:
	    fprintf(stderr, "Trap: Divide by zero.\n");
	    break;
	default:
	    fprintf(stderr, "Trap: Invalid trap code: %d\n", code);
	    break;
    }
    exit(EX_DATAERR);
}


void    init_stats(void)

{
    Stats.instruction_count = 0;
    Stats.clock_cycles = 0;
    Stats.memory_reads = 0;
    Stats.unaligned_reads = 0;
    Stats.memory_writes = 0;
    Stats.unaligned_writes = 0;
    Stats.page_changes = 0;   /* Different page than previous ref */
    Stats.cache_hits = 0;
}

