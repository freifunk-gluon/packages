/*
  Copyright (c) 2017, Jan-Philipp Litza <janphilipp@litza.de>
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


#include "version.h"

#include <ctype.h>
#include <string.h>

static int char_order(char c) {
	if (isdigit(c))
		return 0;
	else if (isalpha(c))
		return c;
	else if (c == '~')
		return -1;
	else
		return c + 256;
}

bool newer_than(const char *a, const char *b) {
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	while (*a != '\0' && *b != '\0') {
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
