// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2012 Nils Schneider <nils@nilsschneider.net>

#include "hexutil.h"

#include <stdio.h>
#include <string.h>


bool parsehex(void *output, const char *input, size_t len) {
	unsigned char *buffer = output;

	// number of digits must be 2 * len
	if ((strspn(input, "0123456789abcdefABCDEF") != 2*len) || input[2*len])
		return false;

	for (size_t i = 0; i < len; i++)
		sscanf(&input[2*i], "%02hhx", &buffer[i]);

	return true;
}
