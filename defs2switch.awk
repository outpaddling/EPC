$2 ~ "OP_" {
    len = length($2);
    opcode = substr($2, 4, len-3);
    printf("\t    case    %s:\n\t\t%s();\n\t\tbreak;\n\n", $2, tolower(opcode));
}

