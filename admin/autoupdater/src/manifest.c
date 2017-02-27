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


#include "hexutil.h"
#include "manifest.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// only frees the data inside the manifest struct, not the struct itself!
void clear_manifest(struct manifest *m) {
	free(m->image_filename);
	free(m->version);

	for (size_t i = 0; i < m->n_signatures; i++)
		free(m->signatures[i]);
	free(m->signatures);

	memset(m, 0, sizeof(*m));
}


static bool parse_rfc3339(const char *input, time_t *date) {
	char tzs;
	unsigned year, month, day, hour, minute, second, tzh, tzm;

	if (sscanf(input, "%04u-%02u-%02u %02u:%02u:%02u%c%02u:%02u",
		   &year, &month, &day, &hour, &minute, &second,
		   &tzs, &tzh, &tzm) != 9)
		return false;

	time_t a = (14 - month)/12;
	time_t y = year - a;
	time_t m = month + 12*a - 3;

	/* Based on a well-known formula for Julian dates */
	time_t days = day + (153*m + 2)/5 + 365*y + y/4 - y/100 + y/400 - 719469;
	time_t tim = hour*3600 + minute*60 + second;


	time_t tz = 3600 * tzh + 60 * tzm;
	if (tzs == '-')
		tz = -tz;
	else if (tzs != '+')
		return false;


	*date = 86400*days + tim - tz;
	return true;

}


void parse_line(char *line, struct manifest *m, const char *branch, const char *image_name) {
	if (m->sep_found) {
		ecdsa_signature_t *sig = malloc(sizeof(ecdsa_signature_t));
		if (!parsehex(sig, line, sizeof(*sig))) {
			free(sig);
			fprintf(stderr, "autoupdater: warning: garbage in signature area: %s\n", line);
			return;
		}
		m->n_signatures++;
		m->signatures = realloc(m->signatures, m->n_signatures * sizeof(ecdsa_signature_t *));
		m->signatures[m->n_signatures - 1] = sig;
	} else if (strcmp(line, "---") == 0) {
		m->sep_found = true;
	} else {
		ecdsa_sha256_update(&m->hash_ctx, line, strlen(line));
		ecdsa_sha256_update(&m->hash_ctx, "\n", 1);

		if (!strncmp(line, "BRANCH=", 7) && !strcmp(&line[7], branch)) {
			m->branch_ok = true;
		}

		else if (!strncmp(line, "DATE=", 5)) {
			if (m->date_ok)
				return;

			m->date_ok = parse_rfc3339(&line[5], &m->date);
		}

		else if (!strncmp(line, "PRIORITY=", 9)) {
			if (m->priority_ok)
				return;

			m->priority = strtof(&line[9], NULL);
			m->priority_ok = true;
		}

		else {
			if (m->model_ok)
				return;

			char *model = strtok(line, " ");
			char *version = strtok(NULL, " ");
			char *checksum = strtok(NULL, " ");
			char *imagesize = strtok(NULL, " ");
			char *filename = strtok(NULL, " ");
			if (!filename || strtok(NULL, " "))
				return;

			if (strcmp(model, image_name) != 0)
				return;

			if (!parsehex(m->image_hash, checksum, ECDSA_SHA256_HASH_SIZE))
				return;

			{

				char *endptr;

				errno = 0;
				unsigned long long val = strtoull(imagesize, &endptr, 10);
				if (errno || *endptr || val > SSIZE_MAX)
					return;

				m->imagesize = val;
			}

			m->version = strdup(version);
			m->image_filename = strdup(filename);

			m->model_ok = true;
		}
	}
}
