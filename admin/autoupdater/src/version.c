// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Jan-Philipp Litza <janphilipp@litza.de>


#include "version.h"

#include <ctype.h>
#include <string.h>

static int char_order(char c) {
	if (isdigit(c))
		return 0;
	else if (isalpha(c))
		return c;
	else if (c == '\0')
		return -1;
	else if (c == '~')
		return -2;
	else
		return c + 256;
}

bool newer_than(const char *a, const char *b) {
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	while (*a != '\0' || *b != '\0') {
		int first_diff = 0;

		// compare non-digits character by character
		while ((*a != '\0' && !isdigit(*a)) || (*b != '\0' && !isdigit(*b))) {
			int ac = char_order(*a);
			int bc = char_order(*b);

			if (ac != bc)
				return ac > bc;

			a++;
			b++;
		}

		// ignore leading zeroes
		while (*a == '0')
			a++;
		while (*b == '0')
			b++;

		// compare numbers digit by digit, but don't return yet in case
		// one number is longer (and thus larger) than the other
		while (isdigit(*a) && isdigit(*b)) {
			if (first_diff == 0)
				first_diff = *a - *b;

			a++;
			b++;
		}

		// check if one number is larger
		if (isdigit(*a))
			return true;
		if (isdigit(*b))
			return false;

		if (first_diff != 0)
			return first_diff > 0;
	}

	return false;
}
