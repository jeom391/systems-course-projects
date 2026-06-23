#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <stdlib.h>
#include <string.h>

void pass1(AssemblerContext *ctx, const AssemblerPaths *paths) {
    FILE *src = fopen(paths->src_path, "r");
    if (!src) {
        return;
    }
    FILE *inter = fopen(paths->int_path, "w");
    if (!inter) {
        fclose(src);
        return;
    }

    ctx->start_address = 0;
    ctx->prog_length = 0;
    strcpy(ctx->program_name, "NONAME");
    strcpy(ctx->exec_label, "-");

    char line[MAX_LINE];
    int locctr = 0;
    int start_set = 0;

    while (fgets(line, sizeof(line), src)) {
        line[strcspn(line, "\r\n")] = '\0';
        size_t idx = 0;
        while (line[idx] == ' ' || line[idx] == '\t') {
            idx++;
        }
        if (line[idx] == '\0' || line[idx] == '.') {
            continue;
        }

        char label[FIELD_LEN];
        char opcode[FIELD_LEN];
        char operand[FIELD_LEN];
        parse_source_fields(line, label, opcode, operand);

        char label_up[FIELD_LEN];
        char opcode_up[FIELD_LEN];
        copy_upper(label_up, label);
        copy_upper(opcode_up, opcode);

        if (!start_set && strcmp(opcode_up, "START") == 0) {
            ctx->start_address = (int)strtol(operand, NULL, 16);
            locctr = ctx->start_address;
            if (label_up[0] != '-') {
                insert_symtab(label_up, locctr);
                strcpy(ctx->program_name, label_up);
            }
            fprintf(inter, "%04X %s %s %s\n", locctr & 0xFFFF, label_up, opcode_up, operand[0] ? operand : "-");
            start_set = 1;
            continue;
        }

        start_set = 1;

        add_literal_if_new(operand);

        if (strcmp(opcode_up, "END") == 0) {
            dump_pool(inter, &locctr);
            if (label_up[0] != '-') {
                insert_symtab(label_up, locctr);
            }
            strcpy(ctx->exec_label, operand);
            fprintf(inter, "%04X %s %s %s\n", locctr & 0xFFFF, label_up, opcode_up, operand[0] ? operand : "-");
            break;
        }

        if (label_up[0] != '-') {
            insert_symtab(label_up, locctr);
        }

        fprintf(inter, "%04X %s %s %s\n", locctr & 0xFFFF, label_up, opcode_up, operand[0] ? operand : "-");

        if (strcmp(opcode_up, "LTORG") == 0) {
            dump_pool(inter, &locctr);
            continue;
        }

        if (find_optab(opcode_up)) {
            locctr += 3;
        } else if (strcmp(opcode_up, "WORD") == 0) {
            locctr += 3;
        } else if (strcmp(opcode_up, "RESW") == 0) {
            locctr += 3 * parse_number(operand);
        } else if (strcmp(opcode_up, "RESB") == 0) {
            locctr += parse_number(operand);
        } else if (strcmp(opcode_up, "BYTE") == 0) {
            locctr += byte_length(operand);
        }
    }

    ctx->prog_length = locctr - ctx->start_address;
    fclose(inter);
    fclose(src);
}
