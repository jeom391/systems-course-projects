#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

#define TABLE_SIZE 31
#define MAX_LINE 256
#define FIELD_LEN 64
#define TEXT_RECORD_MAX_BYTES 30

#define DEFAULT_OPTAB_PATH "examples/optab.txt"
#define DEFAULT_SRC_PATH "examples/srcfile.txt"
#define DEFAULT_INT_PATH "output/intfile.txt"
#define DEFAULT_OBJ_PATH "output/objfile.txt"
#define DEFAULT_OUTPUT_DIR "output"

typedef struct OptNode {
    char mnemonic[FIELD_LEN];
    unsigned int opcode;
    struct OptNode *next;
} OptNode;

typedef struct SymNode {
    char label[FIELD_LEN];
    int address;
    struct SymNode *next;
} SymNode;

typedef struct LitNode {
    char raw[FIELD_LEN];
    char kind;
    char payload[FIELD_LEN];
    int length;
    int address;
    int pooled;
    struct LitNode *next;
} LitNode;

typedef struct {
    int start_address;
    int prog_length;
    char program_name[FIELD_LEN];
    char exec_label[FIELD_LEN];
} AssemblerContext;

typedef struct {
    const char *optab_path;
    const char *src_path;
    const char *int_path;
    const char *obj_path;
    const char *output_dir;
} AssemblerPaths;

typedef struct {
    char buffer[70];
    int start;
    int length;
} TextRecord;

unsigned hash_key(const char *s);
void to_upper(char *s);
void copy_upper(char *dest, const char *src);
int is_hex_digit(char c);
int is_number_token(const char *text);
int parse_number(const char *text);
void ensure_directory(const char *path);

void load_optab(const char *path);
OptNode *find_optab(const char *mnemonic);
void print_optab_dump(void);

SymNode *find_symtab(const char *label);
int insert_symtab(const char *label, int address);
void print_symtab_dump(void);

int parse_literal(const char *operand, char *normalized, char *kind_out, char *payload_out, int *length_out);
void add_literal_if_new(const char *operand);
void dump_pool(FILE *inter, int *locctr);
int find_literal_addr(const char *operand);
void print_littab_dump(void);

int byte_length(const char *operand);
void byte_to_object(const char *operand, char *out);
void parse_source_fields(const char *line, char *label, char *opcode, char *operand);
void parse_intermediate_fields(const char *line, char *loc, char *label, char *opcode, char *operand);

void write_header_record(FILE *obj, const AssemblerContext *ctx);
void text_record_init(TextRecord *record, int start);
void text_record_flush(FILE *obj, TextRecord *record);
void text_record_append(FILE *obj, TextRecord *record, int loc, const char *objcode, int bytes);
void write_end_record(FILE *obj, int exec_address);

void pass1(AssemblerContext *ctx, const AssemblerPaths *paths);
void pass2(const AssemblerContext *ctx, const AssemblerPaths *paths);

#endif
