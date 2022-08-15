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

extern stats_t     Stats;
extern uint8_t     *Mem;
extern uint32_t    *Cache;

int     main(int argc,char *argv[])

{
    uint32_t    entry_address;
    
    switch(argc)
    {
	case    2:
	    if ( (entry_address = load_prog(argv[1])) != -1 )
		return run_prog(entry_address, ECISC);
	    break;
	default:
	    fprintf(stderr,"Usage: %s program\n",argv[0]);
	    return EX_USAGE;
    }
    return EX_OK;
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

int     run_prog(uint32_t entry_address, isa_t isa)

{
    term_t      *terminal;
    int         status;
    
    /* Initialize stats */
    init_stats();
    
    /* Set up terminal */
    terminal = init_term(stdin, stdout, stderr, NULL, 0);
    tpush_tty(terminal);
    tcbreak_mode(terminal);
    
    switch(isa)
    {
	case    ECISC:
	    status = ecisc_run(entry_address);
	    break;
	case    RISC_V:
	    fprintf(stderr, "epc: RISC_V support is not implemented yet.\n");
	    status = EX_SOFTWARE;
	    break;
	default:
	    fprintf(stderr, "epc: Unsupported ISA: %d\n", isa);
	    return EX_SOFTWARE;
    }

    tpop_tty(terminal);
    print_stats(&Stats);
    
    return status;
}


void mem_read_stats(uint32_t address, uint32_t align_mask)

{
    ++Stats.memory_reads;
    // 1 mem cycle for aligned read, 2 if crossing 32-bit word boundary
    int cache_index = ECISC_CACHE_INDEX(address);
    
    if ( Cache[cache_index] == ECISC_CACHED_ADDRESS(address) )
    {
	Stats.clock_cycles += ECISC_CACHE_CLOCKS;
	++Stats.cache_hits;
    }
    else
    {
	Stats.clock_cycles += ECISC_MEM_CLOCKS;
	Cache[cache_index] = address;
    }
    if ( (address & align_mask) != 0 )
    {
	Stats.clock_cycles += ECISC_MEM_CLOCKS;
	++Stats.unaligned_reads;
    }
}


/*
 *  Endian-independent memory access (little-endian regardless of underlying
 *  architecture.
 */

uint32_t read_mem_long(uint32_t address)

{
    mem_read_stats(address, 0x3);
    return Mem[address] \
	| (Mem[address+1] << 8)
	| (Mem[address+2] << 16)
	| (Mem[address+3] << 24);
}


uint16_t read_mem_short(uint32_t address)

{
    mem_read_stats(address, 0x3);
    return Mem[address]
	| (Mem[address+1] << 8);
}


uint8_t  read_mem_byte(uint32_t address)

{
    mem_read_stats(address, 0x0);
    return Mem[address];
}


void mem_write_stats(uint32_t address, uint32_t align_mask)

{
    //printf("Writing %lu\n", address);
    ++Stats.memory_writes;
    // 1 mem cycle for aligned read, 2 if crossing 32-bit word boundary
    Stats.clock_cycles += ECISC_MEM_CLOCKS;
    if ( address & align_mask )
    {
	Stats.clock_cycles += ECISC_MEM_CLOCKS;
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


void write_mem_byte(uint32_t address, uint8_t val)

{
    mem_write_stats(address, 0x0);
    
    Mem[address] = val;
}


float    fetch_float(uint32_t address)

{
    uint32_t    num;
    
    num = read_mem_long(address);
    
    return *(float *)&num;
}


void    store_float(uint32_t address, float num)

{
    write_mem_long(address, *(uint32_t *)&num);
}


void    print_stats(stats_t *stats)

{
    printf("%qu instructions executed.\n", stats->instruction_count);
    // Clock cycles
    /* Not finished */
    printf("%qu clock cycles.\n", stats->clock_cycles);
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
    char        string[EPC_STRING_MAX+1], name[EPC_STRING_MAX+1], *end;
    size_t      len;
    uint32_t    val;
    
    Mem = malloc(ECISC_MEM_SPACE);
    Cache = malloc(ECISC_CACHE_SIZE * 1); /* One entry per 32-bit word */
    
    fp = fopen(filename, "r");
    if ( fp == NULL )
    {
	fprintf(stderr, "%s: Cannot open %s: %s\n",
	    __func__, filename, strerror(errno));
	exit(EX_DATAERR);
    }

    /* Read header */
    /*
    read_string(fp, string, EPC_STRING_MAX);
    if ( strcasecmp(string, "!EPC00") != 0 )
    {
	fputs("This does not appear to be an EPC executable.\n", stderr);
	fputs("Expected !EPC00.\n", stderr);
	exit(EX_DATAERR);
    }
    */
    
    /* Read text segment */
    offset = ECISC_USER_BASE;
    do
    {
	/* Read and discard offset */
	read_string(fp, string, EPC_STRING_MAX);
	
	/* End of program marked by End tag */
	if ( strcasecmp(string, "end") == 0 )
	    break;
	
	/* Read machine code */
	do
	{
	    len = read_string(fp, string, EPC_STRING_MAX);
	    
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
		    val = ECISC_USER_BASE + strtoull(++p, &end, HEX_BASE);
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
    while ( fgets(string, EPC_STRING_MAX, fp) != NULL )
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
	case    ECISC_TRAP_DIV_BY_ZERO:
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

