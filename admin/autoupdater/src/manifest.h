// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Jan-Philipp Litza <janphilipp@litza.de>
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
