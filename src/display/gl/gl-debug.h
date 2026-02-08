/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef DEBUGLOGGER_H
#define DEBUGLOGGER_H

#include "gl-fun.h"

#include <stdio.h>
#include <algorithm>

struct GLDebugLoggerPrivate;

class GLDebugLogger
{
public:
	GLDebugLogger(const char *filename = 0);
	~GLDebugLogger();

private:
	GLDebugLoggerPrivate *p;
};

#define GL_MARKER(format, ...) \
	if (gl.StringMarker) \
	{ \
		char buf[128]; \
		int len = snprintf(buf, sizeof(buf), format, ##__VA_ARGS__); \
		gl.StringMarker(std::min<size_t>(len, sizeof(buf)), buf); \
	}


#endif // DEBUGLOGGER_H
