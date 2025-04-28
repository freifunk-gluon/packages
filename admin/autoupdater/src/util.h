// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Jan-Philipp Litza <janphilipp@litza.de>
#pragma once

#include <stddef.h>


void run_dir(const char *dir);
void randomize(void);
float get_uptime(void);

void * safe_malloc(size_t size);
void * safe_realloc(void *ptr, size_t size);
