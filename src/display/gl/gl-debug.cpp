/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "gl-debug.h"
#include "debugwriter.h"

#include <iostream>

#include "gl-fun.h"

struct GLDebugLoggerPrivate
{
	std::ostream *stream;

	GLDebugLoggerPrivate(const char *logFilename)
	{
		(void) logFilename;

		stream = &std::clog;
	}

	~GLDebugLoggerPrivate()
	{
	}

	void writeTimestamp()
	{
		// FIXME reintroduce proper time stamps (is this even necessary??)
		*stream << "[GLDEBUG] ";
	}

	void writeLine(const char *line)
	{
		*stream << line << "\n";
		stream->flush();
	}
};

static void APIENTRY arbDebugFunc(GLenum source,
                                  GLenum type,
                                  GLuint id,
                                  GLenum severity,
                                  GLsizei length,
                                  const GLchar* message,
                                  const void* userParam)
{
	GLDebugLoggerPrivate *p =
		static_cast<GLDebugLoggerPrivate*>(const_cast<void*>(userParam));

	(void) source;
	(void) type;
	(void) id;
	(void) severity;
	(void) length;

	p->writeTimestamp();
	p->writeLine(message);
}

GLDebugLogger::GLDebugLogger(const char *filename)
{
	p = new GLDebugLoggerPrivate(filename);

	if (gl.DebugMessageCallback)
		gl.DebugMessageCallback(arbDebugFunc, p);
	else
		Debug() << "DebugLogger: no debug extensions found";
}

GLDebugLogger::~GLDebugLogger()
{
	delete p;
}
