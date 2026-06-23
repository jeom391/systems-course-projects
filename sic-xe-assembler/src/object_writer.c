#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <string.h>

void write_header_record(FILE *obj, const AssemblerContext *ctx) {
    char name[7];
    memset(name, 0, sizeof(name));
    size_t i;
    for (i = 0; ctx->program_name[i] && i < 6; i++) {
        name[i] = ctx->program_name[i];
    }
    fprintf(obj, "H %s %06X %06X\n", name, ctx->start_address & 0xFFFFFF, ctx->prog_length & 0xFFFFFF);
}

void text_record_init(TextRecord *record, int start) {
    record->buffer[0] = '\0';
    record->start = start;
    record->length = 0;
}

void text_record_flush(FILE *obj, TextRecord *record) {
    if (record->length > 0) {
        fprintf(obj, "T %06X %02X %s\n", record->start & 0xFFFFFF, record->length & 0xFF, record->buffer);
    }
    record->buffer[0] = '\0';
    record->length = 0;
}

void text_record_append(FILE *obj, TextRecord *record, int loc, const char *objcode, int bytes) {
    if (record->length + bytes > TEXT_RECORD_MAX_BYTES) {
        text_record_flush(obj, record);
    }
    if (record->length == 0) {
        record->start = loc;
    }
    strcat(record->buffer, objcode);
    record->length += bytes;
}

void write_end_record(FILE *obj, int exec_address) {
    fprintf(obj, "E %06X\n", exec_address & 0xFFFFFF);
}

