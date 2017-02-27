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
#pragma once


#include <ecdsautil/ecdsa.h>
#include <ecdsautil/sha256.h>

#include <sys/types.h>
#include <stdbool.h>
#include <time.h>


struct manifest {
	bool sep_found:1;
	bool branch_ok:1;
	bool date_ok:1;
	bool priority_ok:1;
	bool model_ok:1;
	char *image_filename;
	unsigned char *image_hash[ECDSA_SHA256_HASH_SIZE];
	char *version;
	time_t date;
	float priority;
	ssize_t imagesize;

	size_t n_signatures;
	ecdsa_signature_t **signatures;
	ecdsa_sha256_context_t hash_ctx;
};


void clear_manifest(struct manifest *m);

void parse_line(char *line, struct manifest *m, const char *branch, const char *image_name);
