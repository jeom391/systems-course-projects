#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <stdlib.h>
#include <string.h>

static OptNode *optab[TABLE_SIZE];

void load_optab(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return;
    }
    char mnemonic[FIELD_LEN];
    char code[FIELD_LEN];
    while (fscanf(fp, "%63s %63s", mnemonic, code) == 2) {
        to_upper(mnemonic);
        unsigned idx = hash_key(mnemonic);
        OptNode *node = (OptNode *)malloc(sizeof(OptNode));
        strcpy(node->mnemonic, mnemonic);
        node->opcode = (unsigned int)strtoul(code, NULL, 16) & 0xFF;
        node->next = optab[idx];
        optab[idx] = node;
    }
    fclose(fp);
}

OptNode *find_optab(const char *mnemonic) {
    char key[FIELD_LEN];
    copy_upper(key, mnemonic);
    unsigned idx = hash_key(key);
    OptNode *node = optab[idx];
    while (node) {
        if (strcmp(node->mnemonic, key) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

void print_optab_dump(void) {
    printf("\n=== OPTAB ===\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        OptNode *p = optab[i];
        while (p) {
            printf("%-8s : %02X\n", p->mnemonic, p->opcode & 0xFF);
            p = p->next;
        }
    }
}

