// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2015 Matthias Schiffer <mschiffer@universe-factory.net>


#include <libplatforminfo.h>
#include "../common.h"


static char * board_name = NULL;
static char * model = NULL;
static char * image_name = NULL;


__attribute__((constructor)) static void init(void) {
        board_name = read_line("/tmp/sysinfo/board_name");
        model = read_line("/tmp/sysinfo/model");

        sanitize_image_name(&image_name, board_name);
}

__attribute__((destructor)) static void deinit(void) {
        free(board_name);
        free(model);
        free(image_name);

        board_name = NULL;
        model = NULL;
        image_name = NULL;
}


const char * platforminfo_get_board_name(void) {
        return board_name;
}

const char * platforminfo_get_model(void) {
        return model;
}

const char * platforminfo_get_image_name(void) {
        return image_name;
}
