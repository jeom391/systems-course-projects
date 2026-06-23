#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

unsigned hash_key(const char *s) {
    unsigned sum = 0;
    while (*s) {
        sum += (unsigned char)(*s++);
    }
    return sum % TABLE_SIZE;
}

void to_upper(char *s) {
    while (*s) {
        if (*s >= 'a' && *s <= 'z') {
            *s = (char)(*s - 'a' + 'A');
        }
        s++;
    }
}

void copy_upper(char *dest, const char *src) {
    size_t i = 0;
    for (; src[i] && i < FIELD_LEN - 1; i++) {
        char c = src[i];
        if (c >= 'a' && c <= 'z') {
            dest[i] = (char)(c - 'a' + 'A');
        } else {
            dest[i] = c;
        }
    }
    dest[i] = '\0';
}

int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int is_number_token(const char *text) {
    if (text[0] == '-' || text[0] == '\0') {
        return 0;
    }
    char temp[FIELD_LEN];
    copy_upper(temp, text);
    size_t len = strlen(temp);
    size_t start = 0;
    if (len > 2 && temp[0] == '0' && temp[1] == 'X') {
        start = 2;
    }
    size_t end = len;
    if (end > start && temp[end - 1] == 'H') {
        end--;
    }
    if (start >= end) {
        return 0;
    }
    for (size_t i = start; i < end; i++) {
        if (!is_hex_digit(temp[i])) {
            return 0;
        }
    }
    return 1;
}

int parse_number(const char *text) {
    if (text[0] == '-' || text[0] == '\0') {
        return 0;
    }
    char temp[FIELD_LEN];
    copy_upper(temp, text);
    int base = 10;
    size_t len = strlen(temp);
    if (len > 2 && temp[0] == '0' && temp[1] == 'X') {
        base = 16;
        return (int)strtol(temp + 2, NULL, base);
    }
    if (temp[len - 1] == 'H') {
        temp[len - 1] = '\0';
        base = 16;
    } else {
        size_t i;
        for (i = 0; i < len; i++) {
            if (temp[i] >= 'A' && temp[i] <= 'F') {
                base = 16;
                break;
            }
        }
    }
    return (int)strtol(temp, NULL, base);
}

void ensure_directory(const char *path) {
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

