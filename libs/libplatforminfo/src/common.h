/*
  Copyright (c) 2015, Matthias Schiffer <mschiffer@universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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
