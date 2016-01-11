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


#include <libplatforminfo.h>
#include "../common.h"


#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)


static char * model = NULL;


__attribute__((constructor)) static void init(void) {
        FILE *f = fopen("/proc/cpuinfo", "r");
        if (!f)
                return;

        char *line = NULL;
        size_t len = 0;

        while (getline(&line, &len, f) >= 0 && !model) {
                if (strncmp(line, "model name", 10))
                        continue;

                bool colon = false;

                char *p;
                for (p = line + 10; *p; p++) {
                        if (isblank(*p))
                                continue;

                        if (!colon) {
                                if (*p == ':') {
                                        colon = true;
                                        continue;
                                }
                                else {
                                        break;
                                }
                        }

                        size_t len = strlen(p);
                        if (len && p[len-1] == '\n')
                                p[len-1] = 0;

                        model = strdup(p);
                        break;
                }
        }

        free(line);
        fclose(f);
}

__attribute__((destructor)) static void deinit(void) {
        free(model);

        model = NULL;
}


const char * platforminfo_get_board_name(void) {
        return NULL;
}

const char * platforminfo_get_model(void) {
        return model;
}

const char * platforminfo_get_image_name(void) {
#if defined(TARGET_x86_generic)
        return "x86-generic";
#elif defined(TARGET_x86_kvm_guest)
        return "x86-kvm";
#elif defined(TARGET_x86_xen_domu)
        return "x86-xen";
#elif defined(TARGET_x86_64)
        return "x86-64";
#else
#error Unknown x86 subtarget
#endif
}
