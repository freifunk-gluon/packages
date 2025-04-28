// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2015 Matthias Schiffer <mschiffer@universe-factory.net>


#include <libplatforminfo.h>

#include <stddef.h>


#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)


const char * platforminfo_get_target(void) {
	return STRINGIFY(TARGET);
}

const char * platforminfo_get_subtarget(void) {
#ifdef SUBTARGET
	return STRINGIFY(SUBTARGET);
#else
	return NULL;
#endif
}
