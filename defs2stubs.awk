$2 ~ "OP_" {
    len = length($2);
    opcode = substr($2, 4, len-3);
    printf("inline void\t%s(void)\n\n{\n}\n\n", tolower(opcode));
}

