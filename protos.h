/* ecisc.c */
int ecisc_run(uint32_t entry_address);
uint8_t fetch_byte_operand(void);
uint16_t fetch_short_operand(void);
uint32_t get_effective_address(void);
uint32_t fetch_long_operand(void);
float fetch_float_operand(void);
void store_byte_operand(uint8_t operand);
void store_short_operand(uint16_t operand);
void store_long_operand(uint32_t operand);
void store_float_operand(float operand);
void jl(void);
void jeq(void);
void jne(void);
void jlt(void);
void jle(void);
void jltu(void);
void jleu(void);
void jgt(void);
void jge(void);
void jgtu(void);
void jgeu(void);
void jov(void);
void jnv(void);
void j(void);
void movb(void);
void movs(void);
void movl(void);
void movq(void);
void movfl(void);
void movlf(void);
void movfd(void);
void movdf(void);
void cmpb(void);
void cmps(void);
void cmpl(void);
void cmpq(void);
void cmpf(void);
void cmpd(void);
void notb(void);
void nots(void);
void notl(void);
void notq(void);
void andb(void);
void ands(void);
void andl(void);
void andq(void);
void orb(void);
void ors(void);
void orl(void);
void orq(void);
void xorb(void);
void xors(void);
void xorl(void);
void xorq(void);
void sllb(void);
void slls(void);
void slll(void);
void sllq(void);
void srlb(void);
void srls(void);
void srll(void);
void srlq(void);
void srab(void);
void sras(void);
void sral(void);
void sraq(void);
void rolb(void);
void rols(void);
void roll(void);
void rolq(void);
void addb(void);
void adds(void);
void addl(void);
void addq(void);
void addf(void);
void addd(void);
void subb(void);
void subs(void);
void subl(void);
void subq(void);
void subf(void);
void subd(void);
void mulb(void);
void muls(void);
void mull(void);
void mulq(void);
void mulf(void);
void muld(void);
void divb(void);
void divs(void);
void divl(void);
void divq(void);
void divf(void);
void divd(void);
void remb(void);
void rems(void);
void reml(void);
void remq(void);
void inputb(void);
void inputs(void);
void inputl(void);
void outputb(void);
void sleepl(void);
void outputs(void);
void outputl(void);
/* epc.c */
int main(int argc, char *argv[]);
int run_prog(uint32_t entry_address, isa_t isa);
void mem_read_stats(uint32_t address, uint32_t align_mask);
uint32_t read_mem_long(uint32_t address);
uint16_t read_mem_short(uint32_t address);
uint8_t read_mem_byte(uint32_t address);
void mem_write_stats(uint32_t address, uint32_t align_mask);
void write_mem_long(uint32_t address, uint32_t val);
void write_mem_short(uint32_t address, uint16_t val);
void write_mem_byte(uint32_t address, uint8_t val);
float fetch_float(uint32_t address);
void store_float(uint32_t address, float num);
void print_stats(stats_t *stats);
int read_string(FILE *fp, char *string, size_t maxlen);
uint32_t load_prog(char *filename);
void illegal_instruction(uint8_t opcode);
void illegal_addressing_mode(uint8_t mode_byte);
void trap(int code);
void init_stats(void);
/* float_endian.c */
int main(int argc, char *argv[]);
