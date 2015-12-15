#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>


/*
 * batadv-vis updates its originators every 10s
 * We choose a bigger interval to make reboots even more unlikely,
 * and make our interval coprime with batadv-vis's
 */
#define UPDATE_INTERVAL 19

#define INPUT_DIR "/sys/kernel/debug/batman_adv"
#define OUTPUT_DIR "/tmp/batman-adv-visdata"

#define ORIGINATORS "originators"
#define TRANSTABLE_GLOBAL "transtable_global"


static bool copy_file(const char *src, const char *dst) {
        FILE *input = NULL, *output = NULL;

        input = fopen(src, "r");
        if (!input)
                goto error;

        output = fopen(dst, "w");
        if (!output)
                goto error;

        while (!feof(input)) {
                if (ferror(input))
                        goto error;

                char buf[1024];
                size_t r = fread(buf, 1, sizeof(buf), input);
                if (!r)
                        continue;

                if (!fwrite(buf, r, 1, output))
                        goto error;
        }

        fclose(input);
        fclose(output);

        return true;

 error:
        if (input)
                fclose(input);
        if (output)
                fclose(output);

        unlink(dst);
        return false;
}

static void handle_file(const char *mesh, const char *file) {
        char src[strlen(INPUT_DIR) + 1 + strlen(mesh) + 1 + strlen(file) + 1];
        char dst[strlen(OUTPUT_DIR) + 1 + strlen(mesh) + 1 + strlen(file) + 1];
        char tmp[sizeof(dst) + 4];

        snprintf(src, sizeof(src), "%s/%s/%s", INPUT_DIR, mesh, file);
        snprintf(dst, sizeof(dst), "%s/%s/%s", OUTPUT_DIR, mesh, file);
        snprintf(tmp, sizeof(tmp), "%s.new", dst);

        if (copy_file(src, tmp))
                rename(tmp, dst);
        else
                unlink(dst);
}

static void handle_mesh(const char *mesh) {
        char dir[strlen(OUTPUT_DIR) + 1 + strlen(mesh) + 1];

        snprintf(dir, sizeof(dir), "%s/%s", OUTPUT_DIR, mesh);
        mkdir(dir, 0777);

        handle_file(mesh, ORIGINATORS);
        handle_file(mesh, TRANSTABLE_GLOBAL);
}

int main(int argc, char *argv[]) {
        system("rm -rf " OUTPUT_DIR "; mkdir -p " OUTPUT_DIR);

        while (true) {
                int i;
                for (i = 1; i < argc; i++)
                        handle_mesh(argv[i]);

                sleep(UPDATE_INTERVAL);
        }
}
