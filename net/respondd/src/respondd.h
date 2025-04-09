// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2016 Matthias Schiffer <mschiffer@universe-factory.net>


#ifndef _RESPONDD_H_
#define _RESPONDD_H_

typedef struct json_object * (*respondd_provider)(void);

struct respondd_provider_info {
	const char *request;
	const respondd_provider provider;
};

extern const struct respondd_provider_info respondd_providers[];

#endif /* _RESPONDD_H_ */
