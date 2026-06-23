#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <stdlib.h>
#include <string.h>

static LitNode *littab = NULL;

int parse_literal(const char *operand, char *normalized, char *kind_out, char *payload_out, int *length_out) {
    if (!operand || operand[0] != '=') {
        return 0;
    }
    char temp[FIELD_LEN];
    size_t len = strlen(operand);
    if (len >= FIELD_LEN) {
        len = FIELD_LEN - 1;
    }
    strncpy(temp, operand, len);
    temp[len] = '\0';
    while (len > 0 && (temp[len - 1] == ' ' || temp[len - 1] == '\t')) {
        temp[len - 1] = '\0';
        len--;
    }
    size_t idx = 1;
    while (temp[idx] == ' ' || temp[idx] == '\t') {
        idx++;
    }
    const char *body = temp + idx;
    char kind = '\0';
    char payload[FIELD_LEN];
    payload[0] = '\0';
    int length = 0;
    if ((body[0] == 'C' || body[0] == 'c') && body[1] == '\'') {
        kind = 'C';
        const char *end = strchr(body + 2, '\'');
        if (!end) {
            return 0;
        }
        size_t plen = (size_t)(end - (body + 2));
        if (plen > FIELD_LEN - 5) {
            plen = FIELD_LEN - 5;
        }
        strncpy(payload, body + 2, plen);
        payload[plen] = '\0';
        length = (int)plen;
        normalized[0] = '=';
        normalized[1] = kind;
        normalized[2] = '\'';
        memcpy(&normalized[3], payload, plen);
        normalized[3 + plen] = '\'';
        normalized[4 + plen] = '\0';
    } else if ((body[0] == 'X' || body[0] == 'x') && body[1] == '\'') {
        kind = 'X';
        const char *end = strchr(body + 2, '\'');
        if (!end) {
            return 0;
        }
        size_t plen = (size_t)(end - (body + 2));
        if (plen > FIELD_LEN - 5) {
            plen = FIELD_LEN - 5;
        }
        char digits[FIELD_LEN];
        strncpy(digits, body + 2, plen);
        digits[plen] = '\0';
        size_t pos = 0;
        char cleaned[FIELD_LEN];
        cleaned[0] = '\0';
        for (size_t i = 0; digits[i]; i++) {
            char d = digits[i];
            if (d >= 'a' && d <= 'f') {
                d = (char)(d - 'a' + 'A');
            }
            if (!is_hex_digit(d)) {
                return 0;
            }
            cleaned[pos++] = d;
        }
        cleaned[pos] = '\0';
        if (pos % 2 != 0) {
            if (pos + 1 >= FIELD_LEN) {
                return 0;
            }
            for (size_t i = pos; i > 0; i--) {
                cleaned[i] = cleaned[i - 1];
            }
            cleaned[0] = '0';
            pos++;
            cleaned[pos] = '\0';
        }
        size_t max_payload = FIELD_LEN - 5;
        if (pos > max_payload) {
            pos = max_payload;
            if (pos % 2 != 0) {
                pos--;
            }
            cleaned[pos] = '\0';
        }
        strncpy(payload, cleaned, FIELD_LEN - 1);
        payload[FIELD_LEN - 1] = '\0';
        length = (int)(pos / 2);
        normalized[0] = '=';
        normalized[1] = kind;
        normalized[2] = '\'';
        memcpy(&normalized[3], payload, pos);
        normalized[3 + pos] = '\'';
        normalized[4 + pos] = '\0';
    } else if (*body) {
        kind = 'W';
        char number[FIELD_LEN];
        size_t npos = 0;
        while (body[npos] && npos < FIELD_LEN - 1) {
            number[npos] = body[npos];
            npos++;
        }
        number[npos] = '\0';
        while (npos > 0 && (number[npos - 1] == ' ' || number[npos - 1] == '\t')) {
            number[--npos] = '\0';
        }
        if (number[0] == '\0') {
            return 0;
        }
        copy_upper(payload, number);
        size_t plen = strlen(payload);
        if (plen > FIELD_LEN - 2) {
            plen = FIELD_LEN - 2;
            payload[plen] = '\0';
        }
        length = 3;
        normalized[0] = '=';
        memcpy(&normalized[1], payload, plen);
        normalized[1 + plen] = '\0';
    } else {
        return 0;
    }
    if (kind_out) {
        *kind_out = kind;
    }
    if (payload_out) {
        strncpy(payload_out, payload, FIELD_LEN - 1);
        payload_out[FIELD_LEN - 1] = '\0';
    }
    if (length_out) {
        *length_out = length;
    }
    return 1;
}

void add_literal_if_new(const char *operand) {
    char normalized[FIELD_LEN];
    char payload[FIELD_LEN];
    char kind = '\0';
    int length = 0;
    if (!parse_literal(operand, normalized, &kind, payload, &length)) {
        return;
    }
    LitNode *prev = NULL;
    LitNode *node = littab;
    while (node) {
        if (strcmp(node->raw, normalized) == 0) {
            return;
        }
        prev = node;
        node = node->next;
    }
    node = (LitNode *)malloc(sizeof(LitNode));
    strcpy(node->raw, normalized);
    node->kind = kind;
    strcpy(node->payload, payload);
    node->length = length;
    node->address = -1;
    node->pooled = 0;
    node->next = NULL;
    if (prev) {
        prev->next = node;
    } else {
        littab = node;
    }
}

void dump_pool(FILE *inter, int *locctr) {
    LitNode *node = littab;
    while (node) {
        if (!node->pooled) {
            node->address = *locctr;
            if (node->kind == 'W') {
                fprintf(inter, "%04X - WORD %s\n", *locctr & 0xFFFF, node->payload);
                *locctr += 3;
            } else {
                fprintf(inter, "%04X - BYTE %c'%s'\n", *locctr & 0xFFFF, node->kind, node->payload);
                *locctr += node->length;
            }
            node->pooled = 1;
        }
        node = node->next;
    }
}

int find_literal_addr(const char *operand) {
    char normalized[FIELD_LEN];
    char payload[FIELD_LEN];
    char kind = '\0';
    int length = 0;
    if (!parse_literal(operand, normalized, &kind, payload, &length)) {
        return -1;
    }
    LitNode *node = littab;
    while (node) {
        if (strcmp(node->raw, normalized) == 0) {
            return node->address;
        }
        node = node->next;
    }
    return -1;
}

void print_littab_dump(void) {
    printf("\n=== LITTAB ===\n");
    LitNode *p = littab;
    while (p) {
        printf("%-10s kind=%c len=%d addr=%d (0x%04X) pooled=%d payload=%s\n",
               p->raw, p->kind, p->length, p->address, p->address & 0xFFFF, p->pooled, p->payload);
        p = p->next;
    }
}

