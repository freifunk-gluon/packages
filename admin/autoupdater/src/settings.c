/*
  Copyright (c) 2017, Matthias Schiffer <mschiffer@universe-factory.net>
                      Jan-Philipp Litza <janphilipp@litza.de>
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


#include "settings.h"
#include "hexutil.h"

#include <uci.h>

#include <stdlib.h>
#include <string.h>


static char * read_one_line(const char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f)
		return NULL;

	char *line = NULL;
	size_t len = 0;

	ssize_t r = getline(&line, &len, f);

	fclose(f);

	if (r >= 0) {
		len = strlen(line);

		if (len && line[len-1] == '\n')
			line[len-1] = 0;
	}
	else {
		free(line);
		line = NULL;
	}

	return line;
}


static unsigned long load_positive_number(struct uci_context *ctx, struct uci_section *s, const char *option) {
	const char *str = uci_lookup_option_string(ctx, s, option);
	if (!str) {
		fprintf(stderr, "autoupdater: error: unable to load option '%s'\n", option);
		exit(1);
	}

	char *end;
	unsigned long ret = strtoul(str, &end, 0);
	if (*end || !ret) {
		fprintf(stderr, "autoupdater: error: invalid value for option '%s'\n", option);
		exit(1);
	}

	return ret;
}


static const char ** load_string_list(struct uci_context *ctx, struct uci_section *s, const char *option, size_t *len) {
	struct uci_option *o = uci_lookup_option(ctx, s, option);
	if (!o) {
		fprintf(stderr, "autoupdater: error: unable to load option '%s'\n", option);
		exit(1);
	}

	if (o->type != UCI_TYPE_LIST) {
		fprintf(stderr, "autoupdater: error: invalid value for option '%s'\n", option);
		exit(1);
	}

	size_t i = 0;
	struct uci_element *e;
	uci_foreach_element(&o->v.list, e)
		i++;

	*len = i;
	const char **ret = malloc(i * sizeof(char *));

	i = 0;
	uci_foreach_element(&o->v.list, e)
		ret[i++] = e->name;

	return ret;
}


void load_settings(struct settings *settings) {
	struct uci_context *ctx = uci_alloc_context();
	ctx->flags &= ~UCI_FLAG_STRICT;

	struct uci_package *p;
	struct uci_section *s;

	if (uci_load(ctx, "autoupdater", &p) != UCI_OK) {
		fputs("autoupdater: error: unable to load UCI package\n", stderr);
		exit(1);
	}

	s = uci_lookup_section(ctx, p, "settings");
	if (!s || strcmp(s->type, "autoupdater")) {
		fputs("autoupdater: error: unable to load UCI settings\n", stderr);
		exit(1);
	}

	const char *enabled = uci_lookup_option_string(ctx, s, "enabled");
	if ((!enabled || strcmp(enabled, "1")) && !settings->force) {
		fputs("autoupdater is disabled\n", stderr);
		exit(0);
	}

	const char *version_file = uci_lookup_option_string(ctx, s, "version_file");
	if (version_file)
		settings->old_version = read_one_line(version_file);

	if (!settings->branch)
		settings->branch = uci_lookup_option_string(ctx, s, "branch");

	if (!settings->branch) {
		fputs("autoupdater: error: no branch given in settings or command line\n", stderr);
		exit(1);
	}

	struct uci_section *branch = uci_lookup_section(ctx, p, settings->branch);
	if (!branch || strcmp(branch->type, "branch")) {
		fprintf(stderr, "autoupdater: error: unable to load branch configuration for branch '%s'\n", settings->branch);
		exit(1);
	}

	settings->good_signatures = load_positive_number(ctx, branch, "good_signatures");
	if (settings->n_mirrors == 0)
		settings->mirrors = load_string_list(ctx, branch, "mirror", &settings->n_mirrors);

	const char **pubkeys_str = load_string_list(ctx, branch, "pubkey", &settings->n_pubkeys);
	settings->pubkeys = malloc(settings->n_pubkeys * sizeof(ecc_25519_work_t));
	size_t ignored_keys = 0;
	for (size_t i = 0; i < settings->n_pubkeys; i++) {
		ecc_int256_t pubkey_packed;
		if (!pubkeys_str[i])
			goto pubkey_fail;
		if (!parsehex(pubkey_packed.p, pubkeys_str[i], 32))
			goto pubkey_fail;
		if (!ecc_25519_load_packed_legacy(&settings->pubkeys[i-ignored_keys], &pubkey_packed))
			goto pubkey_fail;
		if (!ecdsa_is_valid_pubkey(&settings->pubkeys[i-ignored_keys]))
			goto pubkey_fail;
		continue;

pubkey_fail:
		fprintf(stderr, "autoupdater: warning: ignoring invalid public key %s\n", pubkeys_str[i]);
		ignored_keys++;
	}
	settings->n_pubkeys -= ignored_keys;

	/* Don't free UCI context, we still reference values from it */
}
