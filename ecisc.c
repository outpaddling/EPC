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

uint8_t     *Mem;
uint32_t    *Cache;
uint32_t    Regs[ECISC_CPU_REGS];

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


/***************************************************************************
 *  Use auto-c2man to generate a man page from this comment
 *
 *  Library:
 *      #include <>
 *      -l
 *
 *  Description:
 *  
 *  Arguments:
 *
 *  Returns:
 *
 *  Examples:
 *
 *  Files:
 *
 *  Environment
 *
 *  See also:
 *
 *  History: 
 *  Date        Name        Modification
 *  2022-08-14  Jason Bacon Begin
 ***************************************************************************/

int     ecisc_run(uint32_t entry_address)

{
    uint32_t    opcode;
    
    /* Initialize Program_counter */
    // ECISC_PC = USER_BASE;
    ECISC_PC = ECISC_USER_BASE + entry_address;
    DPRINTF("Starting program at 0x%08X\r\n", ECISC_PC);

    /* Initialize stack */
    ECISC_SP = ECISC_USER_END;

    do
    {
	//DPRINTF("address: %08x ", ECISC_PC);
	opcode = read_mem_byte(ECISC_PC++);   // Mem[ECISC_PC++];
	//DPRINTF("Opcode: %02x\r\n", opcode);
	
	switch( opcode )
	{
	    case    ECISC_OP_JL:
		jl();
		break;

	    case    ECISC_OP_BEQ:
		jeq();
		break;

	    case    ECISC_OP_BNE:
		jne();
		break;

	    case    ECISC_OP_BLT:
		jlt();
		break;

	    case    ECISC_OP_BLE:
		jle();
		break;

	    case    ECISC_OP_BLTU:    /* Same as jnc */
		jltu();
		break;

	    case    ECISC_OP_BLEU:
		jleu();
		break;

	    case    ECISC_OP_BGT:
		jgt();
		break;

	    case    ECISC_OP_BGE:
		jge();
		break;

	    case    ECISC_OP_BGTU:
		jgtu();
		break;

	    case    ECISC_OP_BGEU:    /* Same as joc */
		jgeu();
		break;

	    case    ECISC_OP_BOV:
		jov();
		break;

	    case    ECISC_OP_BNV:
		jnv();
		break;

	    case    ECISC_OP_J:
		j();
		break;

	    case    ECISC_OP_MOVB:
		movb();
		break;

	    case    ECISC_OP_MOVS:
		movs();
		break;

	    case    ECISC_OP_MOVL:    /* Also movf */
		movl();
		break;

	    case    ECISC_OP_MOVQ:    /* Also movd */
		movq();
		break;

	    case    ECISC_OP_MOVFL:
		movfl();
		break;

	    case    ECISC_OP_MOVLF:
		movlf();
		break;

	    case    ECISC_OP_MOVFD:
		movfd();
		break;

	    case    ECISC_OP_MOVDF:
		movdf();
		break;

	    case    ECISC_OP_CMPB:
		cmpb();
		break;

	    case    ECISC_OP_CMPS:
		cmps();
		break;

	    case    ECISC_OP_CMPL:
		cmpl();
		break;

	    case    ECISC_OP_CMPQ:
		cmpq();
		break;

	    case    ECISC_OP_CMPF:
		cmpf();
		break;

	    case    ECISC_OP_CMPD:
		cmpd();
		break;

	    case    ECISC_OP_NOTB:
		notb();
		break;

	    case    ECISC_OP_NOTS:
		nots();
		break;

	    case    ECISC_OP_NOTL:
		notl();
		break;

	    case    ECISC_OP_NOTQ:
		notq();
		break;

	    case    ECISC_OP_ANDB:
		andb();
		break;

	    case    ECISC_OP_ANDS:
		ands();
		break;

	    case    ECISC_OP_ANDL:
		andl();
		break;

	    case    ECISC_OP_ANDQ:
		andq();
		break;

	    case    ECISC_OP_ORB:
		orb();
		break;

	    case    ECISC_OP_ORS:
		ors();
		break;

	    case    ECISC_OP_ORL:
		orl();
		break;

	    case    ECISC_OP_ORQ:
		orq();
		break;

	    case    ECISC_OP_XORB:
		xorb();
		break;

	    case    ECISC_OP_XORS:
		xors();
		break;

	    case    ECISC_OP_XORL:
		xorl();
		break;

	    case    ECISC_OP_XORQ:
		xorq();
		break;

	    case    ECISC_OP_SLLB:
		sllb();
		break;

	    case    ECISC_OP_SLLS:
		slls();
		break;

	    case    ECISC_OP_SLLL:
		slll();
		break;

	    case    ECISC_OP_SLLQ:
		sllq();
		break;

	    case    ECISC_OP_SRLB:
		srlb();
		break;

	    case    ECISC_OP_SRLS:
		srls();
		break;

	    case    ECISC_OP_SRLL:
		srll();
		break;

	    case    ECISC_OP_SRLQ:
		srlq();
		break;

	    case    ECISC_OP_SRAB:
		srab();
		break;

	    case    ECISC_OP_SRAS:
		sras();
		break;

	    case    ECISC_OP_SRAL:
		sral();
		break;

	    case    ECISC_OP_SRAQ:
		sraq();
		break;

	    case    ECISC_OP_ROLB:
		rolb();
		break;

	    case    ECISC_OP_ROLS:
		rols();
		break;

	    case    ECISC_OP_ROLL:
		roll();
		break;

	    case    ECISC_OP_ROLQ:
		rolq();
		break;

	    case    ECISC_OP_ADDB:
		addb();
		break;

	    case    ECISC_OP_ADDS:
		adds();
		break;

	    case    ECISC_OP_ADDL:
		addl();
		break;

	    case    ECISC_OP_ADDQ:
		addq();
		break;

	    case    ECISC_OP_ADDF:
		addf();
		break;

	    case    ECISC_OP_ADDD:
		addd();
		break;

	    case    ECISC_OP_SUBB:
		subb();
		break;

	    case    ECISC_OP_SUBS:
		subs();
		break;

	    case    ECISC_OP_SUBL:
		subl();
		break;

	    case    ECISC_OP_SUBQ:
		subq();
		break;

	    case    ECISC_OP_SUBF:
		subf();
		break;

	    case    ECISC_OP_SUBD:
		subd();
		break;

	    case    ECISC_OP_MULB:
		mulb();
		break;

	    case    ECISC_OP_MULS:
		muls();
		break;

	    case    ECISC_OP_MULL:
		mull();
		break;

	    case    ECISC_OP_MULQ:
		mulq();
		break;

	    case    ECISC_OP_MULF:
		mulf();
		break;

	    case    ECISC_OP_MULD:
		muld();
		break;

	    case    ECISC_OP_DIVB:
		divb();
		break;

	    case    ECISC_OP_DIVS:
		divs();
		break;

	    case    ECISC_OP_DIVL:
		divl();
		break;

	    case    ECISC_OP_DIVQ:
		divq();
		break;

	    case    ECISC_OP_DIVF:
		divf();
		break;

	    case    ECISC_OP_DIVD:
		divd();
		break;

	    case    ECISC_OP_REMB:
		remb();
		break;

	    case    ECISC_OP_REMS:
		rems();
		break;

	    case    ECISC_OP_REML:
		reml();
		break;

	    case    ECISC_OP_REMQ:
		remq();
		break;

	    case    ECISC_OP_INPUTB:
		inputb();
		break;

	    case    ECISC_OP_INPUTS:
		inputs();
		break;

	    case    ECISC_OP_INPUTL:
		inputl();
		break;

	    case    ECISC_OP_OUTPUTB:
		outputb();
		break;

	    case    ECISC_OP_OUTPUTS:
		outputs();
		break;

	    case    ECISC_OP_OUTPUTL:
		outputl();
		break;

	    case    ECISC_OP_SLEEPL:
		sleepl();
		break;
		
	    case    ECISC_OP_HALT:
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
	
    }   while ( opcode != ECISC_OP_HALT ); //opcode != ECISC_OP_HALT );
    
    return EX_OK;
}


uint8_t fetch_byte_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    return Regs[ECISC_REG_NUM(mode_byte)] & ECISC_MASK_LOW_BYTE;
	    
	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return read_mem_byte(address);  // Mem[address];
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)]++;
	    return read_mem_byte(address);  // Mem[address];
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    address = --Regs[ECISC_REG_NUM(mode_byte)];
	    return read_mem_byte(address);  // Mem[address];
	    
	case    ECISC_MODE_OFFSET:
	    offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    /* DPRINTF("offset = %08x  r%d = %08x\n",
		offset, ECISC_REG_NUM(mode_byte), Regs[ECISC_REG_NUM(mode_byte)]); */
	    return read_mem_byte(offset + Regs[ECISC_REG_NUM(mode_byte)]);
	    // Mem[offset + Regs[ECISC_REG_NUM(mode_byte)]];
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return read_mem_byte(address);  // Mem[address];
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    //DPRINTF("pointer = %08x\n", pointer);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    //DPRINTF("address = %08x\n", address);
	    return read_mem_byte(address);  // Mem[address];

	default:
	    fprintf(stderr, "fetch_byte_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


uint16_t fetch_short_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    return Regs[ECISC_REG_NUM(mode_byte)] & ECISC_MASK_LOW_SHORT;
	    
	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return read_mem_short(address);
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    Regs[ECISC_REG_NUM(mode_byte)] += SHORT_BYTES;
	    //DPRINTF("fetch_short: ECISC_PC = %08x\n", ECISC_PC);
	    return read_mem_short(address);
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    Regs[ECISC_REG_NUM(mode_byte)] -= SHORT_BYTES;
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return read_mem_short(address);
	    
	case    ECISC_MODE_OFFSET:
	    offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return read_mem_short(offset + Regs[ECISC_REG_NUM(mode_byte)]);
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return read_mem_short(address);
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return read_mem_short(address);

	default:
	    fprintf(stderr, "fetch_short_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


uint32_t get_effective_address(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_INDIRECT:
	    return Regs[ECISC_REG_NUM(mode_byte)];
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    Regs[ECISC_REG_NUM(mode_byte)] += LONG_BYTES;
	    return address;
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    Regs[ECISC_REG_NUM(mode_byte)] -= LONG_BYTES;
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return address;
	    
	case    ECISC_MODE_OFFSET:
	    offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return offset + Regs[ECISC_REG_NUM(mode_byte)];
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return address;
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return address;

	default:
	    fprintf(stderr, "get_effective_address: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


uint32_t fetch_long_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    return Regs[ECISC_REG_NUM(mode_byte)];
	    
	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return read_mem_long(address);
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    Regs[ECISC_REG_NUM(mode_byte)] += LONG_BYTES;
	    return read_mem_long(address);
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    Regs[ECISC_REG_NUM(mode_byte)] -= LONG_BYTES;
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return read_mem_long(address);
	    
	case    ECISC_MODE_OFFSET:
	    offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return read_mem_long(offset + Regs[ECISC_REG_NUM(mode_byte)]);
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return read_mem_long(address);
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return read_mem_long(address);

	default:
	    fprintf(stderr, "fetch_long_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


float fetch_float_operand(void)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    return ECISC_REG_FLOAT(ECISC_REG_NUM(mode_byte));
	    
	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return fetch_float(address);
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    Regs[ECISC_REG_NUM(mode_byte)] += FLOAT_BYTES;
	    return fetch_float(address);
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    Regs[ECISC_REG_NUM(mode_byte)] -= FLOAT_BYTES;
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    return fetch_float(address);
	    
	case    ECISC_MODE_OFFSET:
	    offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return fetch_float(offset + Regs[ECISC_REG_NUM(mode_byte)]);
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    return fetch_float(address);
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    return fetch_float(address);

	default:
	    fprintf(stderr, "fetch_float_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
    return 0;
}


void store_byte_operand(uint8_t operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, offset, pointer;
    
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    Regs[ECISC_REG_NUM(mode_byte)] = 
		(Regs[ECISC_REG_NUM(mode_byte)] & 0xffffff00) | operand;
	    return;

	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)]++;
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    address = --Regs[ECISC_REG_NUM(mode_byte)];
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	    
	case    ECISC_MODE_OFFSET:
	    offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    write_mem_byte(offset + Regs[ECISC_REG_NUM(mode_byte)], operand);
	    // Mem[offset + Regs[ECISC_REG_NUM(mode_byte)]] = operand;
	    return;
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    write_mem_byte(address, operand);   // Mem[address] = operand;
	    return;

	default:
	    fprintf(stderr, "store_byte_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


void store_short_operand(uint16_t operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, pointer;
    
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    Regs[ECISC_REG_NUM(mode_byte)] = 
		(Regs[ECISC_REG_NUM(mode_byte)] & 0xffff0000) | operand;
	    return;

	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    write_mem_short(address, operand);
	    return;
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    Regs[ECISC_REG_NUM(mode_byte)] += SHORT_BYTES;
	    write_mem_short(address, operand);
	    return;
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    Regs[ECISC_REG_NUM(mode_byte)] -= SHORT_BYTES;
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    write_mem_short(address, operand);
	    return;
	    
	case    ECISC_MODE_OFFSET:
	    //offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    write_mem_short(address, operand);
	    return;
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    write_mem_short(address, operand);
	    return;
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    write_mem_short(address, operand);
	    return;

	default:
	    fprintf(stderr, "store_short_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


void store_long_operand(uint32_t operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, pointer;
    
    //DPRINTF("Storing %08x\n", operand);
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    Regs[ECISC_REG_NUM(mode_byte)] = operand;
	    return;

	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    write_mem_long(address, operand);
	    return;
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    Regs[ECISC_REG_NUM(mode_byte)] += LONG_BYTES;
	    write_mem_long(address, operand);
	    return;
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    Regs[ECISC_REG_NUM(mode_byte)] -= LONG_BYTES;
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    write_mem_long(address, operand);
	    return;
	    
	case    ECISC_MODE_OFFSET:
	    //offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    write_mem_long(address, operand);
	    return;
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    write_mem_long(address, operand);
	    //DPRINTF("Storing %08x at %08x\n", operand, address);
	    return;
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    write_mem_long(address, operand);
	    return;

	default:
	    fprintf(stderr, "store_long_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


void store_float_operand(float operand)

{
    static uint8_t  mode_byte;
    static uint32_t address, pointer;
    
    //DPRINTF("Storing %f\n", operand);
    mode_byte = read_mem_byte(ECISC_PC++);    // Mem[ECISC_PC++];
    switch(ECISC_MODE_BITS(mode_byte))
    {
	case    ECISC_MODE_REG_DIRECT:
	    ECISC_REG_FLOAT(ECISC_REG_NUM(mode_byte)) = operand;
	    return;

	case    ECISC_MODE_REG_INDIRECT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    store_float(address, operand);
	    return;
	    
	case    ECISC_MODE_AUTO_INCREMENT:
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    Regs[ECISC_REG_NUM(mode_byte)] += FLOAT_BYTES;
	    store_float(address, operand);
	    return;
	    
	case    ECISC_MODE_AUTO_DECREMENT:
	    Regs[ECISC_REG_NUM(mode_byte)] -= FLOAT_BYTES;
	    address = Regs[ECISC_REG_NUM(mode_byte)];
	    store_float(address, operand);
	    return;
	    
	case    ECISC_MODE_OFFSET:
	    //offset = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    store_float(address, operand);
	    return;
	
	case    ECISC_MODE_MEM_DIRECT:
	    address = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    store_float(address, operand);
	    //DPRINTF("Storing %08x at %08x\n", operand, address);
	    return;
	
	case    ECISC_MODE_MEM_INDIRECT:
	    pointer = read_mem_long(ECISC_PC);
	    ECISC_PC += ADDRESS_BYTES;
	    address = read_mem_long(pointer);
	    store_float(address, operand);
	    return;

	default:
	    fprintf(stderr, "store_float_operand: ");
	    illegal_addressing_mode(mode_byte);
    }
}


void jl(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    /* Push ECISC_PC */
    ECISC_SP -= ADDRESS_BYTES;
    write_mem_long(ECISC_SP, ECISC_PC);
    
    /* Branch */
    ECISC_PC = address;
}


void jeq(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.z == 1 )
	ECISC_PC = address;
}


void jne(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.z == 0 )
	ECISC_PC = address;
}


void jlt(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.n == 1 )
	ECISC_PC = address;
}


void jle(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.n == 1) || (Status.z == 1) )
	ECISC_PC = address;
}


void jltu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.c == 0 )
	ECISC_PC = address;
}


void jleu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.c == 0) || (Status.z == 1) )
	ECISC_PC = address;
}


void jgt(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.n == 0) && (Status.z == 0) )
	ECISC_PC = address;
}


void jge(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.n == 0) || (Status.z == 1) )
	ECISC_PC = address;
}


void jgtu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( (Status.c == 1) && (Status.z == 0) )
	ECISC_PC = address;
}


void jgeu(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.c == 1 )
	ECISC_PC = address;
}


void jov(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.v == 1 )
	ECISC_PC = address;
}


void jnv(void)

{
    static uint32_t address;
    
    address = get_effective_address();
    
    if ( Status.v == 0 )
	ECISC_PC = address;
}


void j(void)

{
    ECISC_PC = get_effective_address();
}


void movb(void)

{
    store_byte_operand(fetch_byte_operand());
}


void movs(void)

{
    store_short_operand(fetch_short_operand());
}


void movl(void)

{
    store_long_operand(fetch_long_operand());
}


void movq(void)

{
    fprintf(stderr, "movq not yet implemented.\n");
}


void     movfl(void)

{
    store_long_operand((long)fetch_float_operand());
}


void     movlf(void)

{
    store_float_operand((float)fetch_long_operand());
}


void     movfd(void)

{
    fprintf(stderr, "movdf not yet implemented.\n");
}


void     movdf(void)

{
    fprintf(stderr, "movdf not yet implemented.\n");
}


void cmpb(void)

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


void     cmps(void)

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


void     cmpl(void)

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


void     cmpq(void)

{
    fprintf(stderr, "cmpq not yet implemented.\n");
}


void     cmpf(void)

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


void     cmpd(void)

{
    fprintf(stderr, "cmpd not yet implemented.\n");
}


void     notb(void)

{
    store_byte_operand(~fetch_byte_operand());
}


void     nots(void)

{
    store_short_operand(~fetch_short_operand());
}


void     notl(void)

{
    store_long_operand(~fetch_long_operand());
}


void     notq(void)

{
    fprintf(stderr, "notq not yet implemented.\n");
}


void     andb(void)

{
    store_byte_operand(fetch_byte_operand() & fetch_byte_operand());
}


void     ands(void)

{
    store_short_operand(fetch_short_operand() & fetch_short_operand());
}


void     andl(void)

{
    store_long_operand(fetch_long_operand() & fetch_long_operand());
}


void     andq(void)

{
    fprintf(stderr, "andq not yet implemented.\n");
}


void     orb(void)

{
    store_byte_operand(fetch_byte_operand() | fetch_byte_operand());
}


void     ors(void)

{
    store_short_operand(fetch_short_operand() & fetch_short_operand());
}


void     orl(void)

{
    store_long_operand(fetch_long_operand() & fetch_long_operand());
}


void     orq(void)

{
    fprintf(stderr, "orq not yet implemented.\n");
}


void     xorb(void)

{
    store_byte_operand(fetch_byte_operand() ^ fetch_byte_operand());
}


void     xors(void)

{
    store_short_operand(fetch_short_operand() ^ fetch_short_operand());
}


void     xorl(void)

{
    store_long_operand(fetch_long_operand() ^ fetch_long_operand());
}


void     xorq(void)

{
    fprintf(stderr, "xorq not yet implemented.\n");
}


void     sllb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint8_t operand1 = fetch_byte_operand();  /* Byte to shift */
    
    /* Second operand is shift count */
    store_byte_operand(operand1 << fetch_byte_operand());
}


void     slls(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint16_t operand1 = fetch_short_operand();  /* Short to shift */
    
    /* Second operand is shift count */
    store_short_operand(operand1 << fetch_short_operand());
}


void     slll(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint32_t operand1 = fetch_long_operand();  /* Long to shift */
    
    /* Second operand is shift count */
    store_long_operand(operand1 << fetch_long_operand());
}


void     sllq(void)

{
    fprintf(stderr, "sllq not yet implemented.\n");
}


void     srlb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint8_t operand1 = fetch_byte_operand();  /* Byte to shift */
    
    /* Second operand is shift count */
    store_byte_operand(operand1 >> fetch_byte_operand());
}


void     srls(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    uint16_t operand1 = fetch_short_operand();  /* Short to shift */
    
    /* Second operand is shift count */
    store_short_operand(operand1 >> fetch_short_operand());
}


void     srll(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
   
    uint32_t operand1 = fetch_long_operand();  /* Long to shift */
    
    /* Second operand is shift count */
    store_long_operand(operand1 >> fetch_long_operand());
}


void     srlq(void)

{
    fprintf(stderr, "srlq not yet implemented.\n");
}


void     srab(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t operand1 = fetch_byte_operand();  /* Byte to shift */
    
    /* Second operand is shift count */
    store_byte_operand(operand1 >> fetch_byte_operand());
}


void     sras(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t operand1 = fetch_short_operand();  /* Short to shift */
    
    /* Second operand is shift count */
    store_short_operand(operand1 >> fetch_short_operand());
}


void     sral(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int32_t operand1 = fetch_long_operand();  /* Long to shift */
    
    /* Second operand is shift count */
    store_long_operand(operand1 >> fetch_long_operand());
}


void     sraq(void)

{
    fprintf(stderr, "sraq not yet implemented.\n");
}


void     rolb(void)

{
    fprintf(stderr, "rolb not yet implemented.\n");
}


void     rols(void)

{
    fprintf(stderr, "rols not yet implemented.\n");
}


void     roll(void)

{
    fprintf(stderr, "roll not yet implemented.\n");
}


void     rolq(void)

{
    fprintf(stderr, "rolq not yet implemented.\n");
}


void     addb(void)

{
    store_byte_operand(fetch_byte_operand() + fetch_byte_operand());
}


void     adds(void)

{
    store_short_operand(fetch_short_operand() + fetch_short_operand());
}


void     addl(void)

{
    store_long_operand(fetch_long_operand() + fetch_long_operand());
}


void     addq(void)

{
    fprintf(stderr, "addq not yet implemented.\n");
}


void     addf(void)

{
    static float    operand1, operand2;
    
    operand1 = fetch_float_operand();
    operand2 = fetch_float_operand();
    //DPRINTF("addf: %f + %f = %f\n", operand1, operand2, operand1 + operand2);
    store_float_operand(operand1 + operand2);
}


void     addd(void)

{
    fprintf(stderr, "addd not yet implemented.\n");
}


void subb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t operand1 = fetch_byte_operand();
    
    /* Second operand is shift count */
    store_byte_operand(operand1 - fetch_byte_operand());
}


void subs(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t operand1 = fetch_short_operand();
    
    /* Second operand is shift count */
    store_short_operand(operand1 - fetch_short_operand());
}


void subl(void)

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


void     subq(void)

{
    fprintf(stderr, "subq not yet implemented.\n");
}


void     subf(void)

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


void     subd(void)

{
    fprintf(stderr, "subd not yet implemented.\n");
}


void     mulb(void)

{
    store_byte_operand(fetch_byte_operand() * fetch_byte_operand());
}


void     muls(void)

{
    store_short_operand(fetch_short_operand() * fetch_short_operand());
}


void     mull(void)

{
    store_long_operand(fetch_long_operand() * fetch_long_operand());
}


void     mulq(void)

{
    fprintf(stderr, "mulq not yet implemented.\n");
}


void     mulf(void)

{
    store_float_operand(fetch_float_operand() * fetch_float_operand());
}


void     muld(void)

{
    fprintf(stderr, "muld not yet implemented.\n");
}


void     divb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t dividend = fetch_byte_operand();
    int8_t divisor = fetch_byte_operand();
    
    if ( divisor == 0 )
	trap(ECISC_TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_byte_operand(dividend / divisor);
}


void     divs(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t dividend = fetch_short_operand();
    int16_t divisor = fetch_short_operand();
    
    if ( divisor == 0 )
	trap(ECISC_TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_short_operand(dividend / divisor);
}


void     divl(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int32_t dividend = fetch_long_operand();
    int32_t divisor = fetch_long_operand();
    
    if ( divisor == 0 )
	trap(ECISC_TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_long_operand(dividend / divisor);
}


void     divq(void)

{
    fprintf(stderr, "divq not yet implemented.\n");
}


void     divf(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    float   dividend = fetch_float_operand();
    float   divisor = fetch_float_operand();
    
    if ( divisor == 0.0 )
	trap(ECISC_TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_float_operand(dividend / divisor);
}


void     divd(void)

{
    fprintf(stderr, "divd not yet implemented.\n");
}


void     remb(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int8_t dividend = fetch_byte_operand();
    int8_t divisor = fetch_byte_operand();
    
    if ( divisor == 0 )
	trap(ECISC_TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_byte_operand(dividend % divisor);
}


void     rems(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int16_t dividend = fetch_short_operand();
    int16_t divisor = fetch_short_operand();
    
    if ( divisor == 0 )
	trap(ECISC_TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_short_operand(dividend % divisor);
}


void     reml(void)

{
    /*
     *  Order of operands matters here, and order of evaluation
     *  is unknown, so fetch all but one operand in separate statements.
     */
     
    int32_t dividend = fetch_long_operand();
    int32_t divisor = fetch_long_operand();
    
    if ( divisor == 0 )
	trap(ECISC_TRAP_DIV_BY_ZERO);
    
    /* Second operand is shift count */
    store_long_operand(dividend % divisor);
}


void     remq(void)

{
    fprintf(stderr, "remq not yet implemented.\n");
}


void     inputb(void)

{
    /* First operand is I/O address */
    switch(fetch_byte_operand())
    {
	case    ECISC_IO_SERIAL1:
	    store_byte_operand(getchar());
	    break;
	default:
	    break;
    }
}


void     inputs(void)

{
    fprintf(stderr, "inputs not yet implemented.\n");
}


void     inputl(void)

{
    fprintf(stderr, "inputl not yet implemented.\n");
}


void outputb(void)

{
    static uint8_t operand1;
    
    operand1 = fetch_byte_operand();    /* Byte to output */
    
    /* Second operand is I/O address */
    switch(fetch_byte_operand())
    {
	case    ECISC_IO_SERIAL1:
	    putchar(operand1);
	    fflush(stdout);
	    break;
	default:
	    break;
    }
}


void sleepl(void)

{
    usleep(fetch_long_operand());
}


void     outputs(void)

{
    fprintf(stderr, "outputs not yet implemented.\n");
}


void     outputl(void)

{
    fprintf(stderr, "outputl not yet implemented.\n");
}

