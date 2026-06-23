#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <stdlib.h>
#include <string.h>

static SymNode *symtab[TABLE_SIZE];

SymNode *find_symtab(const char *label) {
    char key[FIELD_LEN];
    copy_upper(key, label);
    unsigned idx = hash_key(key);
    SymNode *node = symtab[idx];
    while (node) {
        if (strcmp(node->label, key) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

int insert_symtab(const char *label, int address) {
    SymNode *existing = find_symtab(label);
    if (existing) {
        printf("Error: Duplicate label %s\n", label);
        return 0;
    }
    char key[FIELD_LEN];
    copy_upper(key, label);
    unsigned idx = hash_key(key);
    SymNode *node = (SymNode *)malloc(sizeof(SymNode));
    strcpy(node->label, key);
    node->address = address;
    node->next = symtab[idx];
    symtab[idx] = node;
    return 1;
}

void print_symtab_dump(void) {
    printf("\n=== SYMTAB ===\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        SymNode *p = symtab[i];
        while (p) {
            printf("%-8s : %d (0x%04X)\n", p->label, p->address, p->address & 0xFFFF);
            p = p->next;
        }
    }
}

