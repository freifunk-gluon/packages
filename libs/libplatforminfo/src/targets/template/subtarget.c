// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2015 Matthias Schiffer <mschiffer@universe-factory.net>


#include <libplatforminfo.h>
#include "../common.h"


#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)


static char * model = NULL;


__attribute__((constructor)) static void init(void) {
	model = read_line("/tmp/sysinfo/model");
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
	return STRINGIFY(TARGET) "-" STRINGIFY(SUBTARGET);
}
