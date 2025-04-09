// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Matthias Schiffer <mschiffer@universe-factory.net>
// SPDX-FileCopyrightText: 2017 Jan-Philipp Litza <janphilipp@litza.de>
#pragma once


#include <ecdsautil/ecdsa.h>


struct settings {
	bool force;
	bool fallback;
	bool no_action;
	bool force_version;
	const char *branch;
	unsigned long good_signatures;
	char *old_version;

	size_t n_mirrors;
	const char **mirrors;

	size_t n_pubkeys;
	ecc_25519_work_t *pubkeys;
};


void load_settings(struct settings *settings);
