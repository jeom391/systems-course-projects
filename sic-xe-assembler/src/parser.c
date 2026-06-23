#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <string.h>

int byte_length(const char *operand) {
    if (operand[0] == '\0' || operand[0] == '-') {
        return 0;
    }
    if ((operand[0] == 'C' || operand[0] == 'c') && operand[1] == '\'') {
        int len = 0;
        int i = 2;
        while (operand[i] && operand[i] != '\'') {
            len++;
            i++;
        }
        return len;
    }
    if ((operand[0] == 'X' || operand[0] == 'x') && operand[1] == '\'') {
        int digits = 0;
        int i = 2;
        while (operand[i] && operand[i] != '\'') {
            digits++;
            i++;
        }
        return digits / 2;
    }
    return 1;
}

void byte_to_object(const char *operand, char *out) {
    if ((operand[0] == 'C' || operand[0] == 'c') && operand[1] == '\'') {
        int i = 2;
        int pos = 0;
        while (operand[i] && operand[i] != '\'') {
            unsigned char v = (unsigned char)operand[i];
            sprintf(out + pos, "%02X", v);
            pos += 2;
            i++;
        }
        out[pos] = '\0';
        return;
    }
    if ((operand[0] == 'X' || operand[0] == 'x') && operand[1] == '\'') {
        int i = 2;
        int pos = 0;
        while (operand[i] && operand[i] != '\'') {
            char c = operand[i];
            if (c >= 'a' && c <= 'f') {
                c = (char)(c - 'a' + 'A');
            }
            out[pos++] = c;
            i++;
        }
        out[pos] = '\0';
        return;
    }
    sprintf(out, "%02X", parse_number(operand) & 0xFF);
}

void parse_source_fields(const char *line, char *label, char *opcode, char *operand) {
    label[0] = '-';
    label[1] = '\0';
    opcode[0] = '-';
    opcode[1] = '\0';
    operand[0] = '-';
    operand[1] = '\0';

    char temp[MAX_LINE];
    strcpy(temp, line);
    char *tok = strtok(temp, " \t");
    if (!tok) {
        return;
    }
    if (line[0] != ' ' && line[0] != '\t') {
        strncpy(label, tok, FIELD_LEN - 1);
        label[FIELD_LEN - 1] = '\0';
        tok = strtok(NULL, " \t");
        if (!tok) {
            return;
        }
    }
    strncpy(opcode, tok, FIELD_LEN - 1);
    opcode[FIELD_LEN - 1] = '\0';
    tok = strtok(NULL, " \t");
    if (tok) {
        strncpy(operand, tok, FIELD_LEN - 1);
        operand[FIELD_LEN - 1] = '\0';
    }
}

void parse_intermediate_fields(const char *line, char *loc, char *label, char *opcode, char *operand) {
    loc[0] = '-';
    loc[1] = '\0';
    label[0] = '-';
    label[1] = '\0';
    opcode[0] = '-';
    opcode[1] = '\0';
    operand[0] = '-';
    operand[1] = '\0';

    char temp[MAX_LINE];
    strcpy(temp, line);
    char *tok = strtok(temp, " \t");
    int count = 0;
    while (tok) {
        if (count == 0) {
            strncpy(loc, tok, FIELD_LEN - 1);
            loc[FIELD_LEN - 1] = '\0';
        } else if (count == 1) {
            strncpy(label, tok, FIELD_LEN - 1);
            label[FIELD_LEN - 1] = '\0';
        } else if (count == 2) {
            strncpy(opcode, tok, FIELD_LEN - 1);
            opcode[FIELD_LEN - 1] = '\0';
        } else if (count == 3) {
            strncpy(operand, tok, FIELD_LEN - 1);
            operand[FIELD_LEN - 1] = '\0';
            break;
        }
        count++;
        tok = strtok(NULL, " \t");
    }
}

