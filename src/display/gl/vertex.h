/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef VERTEX_H
#define VERTEX_H

#include "etc-internal.h"
#include "gl-fun.h"
#include "shader.h"

/* Simple Vertex */
struct SVertex
{
	Vec2 pos;
	Vec2 texPos;
};

/* Color Vertex */
struct CVertex
{
	Vec2 pos;
	Vec4 color;

	CVertex();
};

struct Vertex
{
	Vec2 pos;
	Vec2 texPos;
	Vec4 color;

	Vertex();
};

struct VertexAttribute
{
	Shader::Attribute index;
	GLint size;
	GLenum type;
	const GLvoid *offset;
};

template<class VertType>
struct VertexTraits
{
	static const VertexAttribute *attr;
	static const GLsizei attrCount;
};

#endif // VERTEX_H
