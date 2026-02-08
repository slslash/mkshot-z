/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef SERIALUTIL_H
#define SERIALUTIL_H

#include <stdint.h>
#include <string.h>

#include <SDL_endian.h>

#if SDL_BYTEORDER != SDL_LIL_ENDIAN
#error "Non little endian systems not supported"
#endif

static inline int32_t
readInt32(const char **dataP)
{
	int32_t result;

	memcpy(&result, *dataP, 4);
	*dataP += 4;

	return result;
}

static inline double
readDouble(const char **dataP)
{
	double result;

	memcpy(&result, *dataP, 8);
	*dataP += 8;

	return result;
}

static inline void
writeInt32(char **dataP, int32_t value)
{
	memcpy(*dataP, &value, 4);
	*dataP += 4;
}

static inline void
writeDouble(char **dataP, double value)
{
	memcpy(*dataP, &value, 8);
	*dataP += 8;
}

#endif // SERIALUTIL_H
