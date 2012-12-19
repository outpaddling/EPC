/* epc.c */
int main(int argc, char *argv[]);
void write_mem_long(uint32_t address, uint32_t val);
void write_mem_short(uint32_t address, uint16_t val);
void store_float(uint32_t address, float num);
int run_prog(uint32_t entry_address);
void print_stats(stats_t *stats);
int read_string(FILE *fp, char *string, size_t maxlen);
uint32_t load_prog(char *filename);
void illegal_instruction(uint8_t opcode);
void illegal_addressing_mode(uint8_t mode_byte);
void trap(int code);
void init_stats(void);
/* float_endian.c */
int main(int argc, char *argv[]);
