#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "maze.h"

static void print_quoted_char(FILE *file,
                              unsigned char c, unsigned char next) {
    if (c == '"' || c == '\\') {
        fprintf(file, "\\");
    }

    if (isprint(c)) {
        fputc(c, file);
        return;
    }

    switch (c) {
    case '\a':
        fprintf(file, "\\a");
        return;
    case '\b':
        fprintf(file, "\\b");
        return;
    case '\f':
        fprintf(file, "\\f");
        return;
    case '\n':
        fprintf(file, "\\n");
        return;
    case '\r':
        fprintf(file, "\\r");
        return;
    case '\t':
        fprintf(file, "\\t");
        return;
    case '\v':
        fprintf(file, "\\v");
        return;
    }

    if (c == '\0') {
        if (isdigit(next) && next < '8') {
            fprintf(file, "\\000");
            return;
        }

        fprintf(file, "\\0");
        return;
    }

    if (isxdigit(next)) {
        unsigned char low  = '0' + (c & 7);
        unsigned char med  = '0' + ((c >> 3) & 7);
        unsigned char high = '0' + ((c >> 6) & 7);
        fprintf(file, "\\%c%c%c", high, med, low);
        return;
    }

    fprintf(file, "\\x%.2X", c);
}

static void print_quoted_str(FILE *file, const char *str) {
    fprintf(file, "\"");

    char prev = '\0';
    for (; *str != '\0'; ++str) {
        if (*str == '?' && prev == '?') {
            switch (str[1]) {
            case '<':
            case '>':
            case '(':
            case ')':
            case '=':
            case '/':
            case '\'':
            case '!':
            case '-':
                fprintf(file, "\\?");
                continue;
            }
        }

        print_quoted_char(file,
                          (unsigned char)*str, (unsigned char)str[1]);
        prev = *str;
    }

    fprintf(file, "\"");
}


int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [seed]\n",
                argc > 0 ? argv[0] : "maze");
        return EXIT_FAILURE;
    }

    if (argc > 1) {
        if (*argv[1] == '\0') {
            fprintf(stderr, "Invalid seed \"\"\n");
            return EXIT_FAILURE;
        }

        char *end;
        errno = 0;
        unsigned long seed = strtoul(argv[1], &end, 0);
        if (*end != '\0') {
            fprintf(stderr, "Invalid seed ");
            print_quoted_str(stderr, argv[1]);
            fprintf(stderr, "\n");
            return EXIT_FAILURE;
        }

        if (errno == ERANGE || seed > UINT_MAX) {
            fprintf(stderr, "Seed %lu is not in range [0, %u]\n",
                    seed, UINT_MAX);
            return EXIT_FAILURE;
        }

        srand((unsigned int)seed);
    } else {
        srand((unsigned int)time(NULL));
    }

    Maze maze = MAZE_INIT;

    maze_randomize(&maze);
    maze_print(&maze);
    return EXIT_SUCCESS;
}
