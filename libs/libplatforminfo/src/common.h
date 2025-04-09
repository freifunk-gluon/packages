// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2014 Matthias Schiffer <mschiffer@universe-factory.net>


#pragma once


#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


__attribute__((unused)) static char * read_line(const char *filename) {
        FILE *f = fopen(filename, "r");
        if (!f)
                return NULL;

        char *line = NULL;
        size_t len = 0;

        ssize_t r = getline(&line, &len, f);

        fclose(f);

        if (r >= 0) {
                len = strlen(line);

                if (len && line[len-1] == '\n')
                        line[len-1] = 0;
        }
        else {
                free(line);
                line = NULL;
        }

        return line;
}

__attribute__((unused)) static void sanitize_image_name(char **outp, char *in) {
        if (!in) {
                *outp = NULL;
                return;
        }

        char *out = malloc(strlen(in) + 1);
        *outp = out;

        bool dot = false, dash = false;

        for (; *in; in++) {
                if (*in == '.') {
                        dot = true;
                        continue;
                }

                if (*in != '+' && !isalnum(*in)) {
                        dash = true;
                        continue;
                }

                if (dash)
                        *out++ = '-';
                else if (dot)
                        *out++ = '.';

                dash = false;
                dot = false;

                *out++ = tolower(*in);
        }

        *out = 0;
}
