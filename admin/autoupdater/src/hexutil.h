// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2012 Nils Schneider <nils@nilsschneider.net>
#pragma once

#include <stdbool.h>
#include <stddef.h>


/* Converts a string of hexadecimal digits and stores it in a given buffer.
 * In order for this function to return successfully the decoded string
 * must fit exactly into the buffer.
 */
bool parsehex(void *buffer, const char *string, size_t len);
