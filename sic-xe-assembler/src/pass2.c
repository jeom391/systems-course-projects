#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <stdlib.h>
#include <string.h>

void pass2(const AssemblerContext *ctx, const AssemblerPaths *paths) {
    FILE *inter = fopen(paths->int_path, "r");
    if (!inter) {
        return;
    }
    FILE *obj = fopen(paths->obj_path, "w");
    if (!obj) {
        fclose(inter);
        return;
    }

    write_header_record(obj, ctx);

    char line[MAX_LINE];
    TextRecord text;
    text_record_init(&text, ctx->start_address);
    int exec_address = ctx->start_address;

    while (fgets(line, sizeof(line), inter)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }

        char loc_str[FIELD_LEN];
        char label[FIELD_LEN];
        char opcode[FIELD_LEN];
        char operand[FIELD_LEN];
        parse_intermediate_fields(line, loc_str, label, opcode, operand);

        int loc = (int)strtol(loc_str, NULL, 16);

        if (strcmp(opcode, "START") == 0) {
            continue;
        }

        if (strcmp(opcode, "LTORG") == 0) {
            text_record_flush(obj, &text);
            continue;
        }

        if (strcmp(opcode, "END") == 0) {
            if (operand[0] != '-' && operand[0] != '\0') {
                SymNode *sym = find_symtab(operand);
                if (sym) {
                    exec_address = sym->address;
                } else {
                    exec_address = parse_number(operand);
                }
            }
            text_record_flush(obj, &text);
            break;
        }

        if (strcmp(opcode, "RESB") == 0 || strcmp(opcode, "RESW") == 0) {
            text_record_flush(obj, &text);
            continue;
        }

        if (strcmp(opcode, "BYTE") == 0) {
            char objcode[FIELD_LEN];
            byte_to_object(operand, objcode);
            int bytes = (int)strlen(objcode) / 2;
            text_record_append(obj, &text, loc, objcode, bytes);
            continue;
        }

        if (strcmp(opcode, "WORD") == 0) {
            char objcode[FIELD_LEN];
            int value = parse_number(operand);
            sprintf(objcode, "%06X", value & 0xFFFFFF);
            text_record_append(obj, &text, loc, objcode, 3);
            continue;
        }

        OptNode *op = find_optab(opcode);
        if (op) {
            char operand_copy[FIELD_LEN];
            strncpy(operand_copy, operand, FIELD_LEN - 1);
            operand_copy[FIELD_LEN - 1] = '\0';

            unsigned int address = 0;
            unsigned int xflag = 0;
            if (operand_copy[0] != '-' && operand_copy[0] != '\0') {
                char *comma = strchr(operand_copy, ',');
                if (comma) {
                    *comma = '\0';
                    char index_part[FIELD_LEN];
                    strncpy(index_part, comma + 1, FIELD_LEN - 1);
                    index_part[FIELD_LEN - 1] = '\0';
                    to_upper(index_part);
                    if (strcmp(index_part, "X") == 0) {
                        xflag = 0x8000;
                    }
                }
                if (operand_copy[0] == '=') {
                    int litaddr = find_literal_addr(operand_copy);
                    if (litaddr >= 0) {
                        address = (unsigned int)litaddr;
                    } else {
                        printf("Error: Undefined literal %s\n", operand_copy);
                        address = 0;
                    }
                } else {
                    SymNode *sym = find_symtab(operand_copy);
                    if (sym) {
                        address = (unsigned int)sym->address;
                    } else if (is_number_token(operand_copy)) {
                        address = (unsigned int)parse_number(operand_copy);
                    } else {
                        printf("Error: Undefined symbol %s\n", operand_copy);
                        address = 0;
                    }
                }
            }
            unsigned int object = ((op->opcode & 0xFF) << 16) | (xflag | (address & 0x7FFF));
            char objcode[FIELD_LEN];
            sprintf(objcode, "%06X", object & 0xFFFFFF);
            text_record_append(obj, &text, loc, objcode, 3);
        }
    }

    text_record_flush(obj, &text);
    write_end_record(obj, exec_address);

    fclose(obj);
    fclose(inter);
}

