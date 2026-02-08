/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "vertex.h"
#include "util.h"

#include <cstddef>

CVertex::CVertex()
    : color(1, 1, 1, 1)
{}

Vertex::Vertex()
    : color(1, 1, 1, 1)
{}

#define o(type, mem) ((const GLvoid*) offsetof(type, mem))

static const VertexAttribute SVertexAttribs[] =
{
	{ Shader::Position, 2, GL_FLOAT, o(SVertex, pos)    },
	{ Shader::TexCoord, 2, GL_FLOAT, o(SVertex, texPos) }
};

static const VertexAttribute CVertexAttribs[] =
{
	{ Shader::Color,    4, GL_FLOAT, o(CVertex, color) },
	{ Shader::Position, 2, GL_FLOAT, o(CVertex, pos)   }
};

static const VertexAttribute VertexAttribs[] =
{
	{ Shader::Color,    4, GL_FLOAT, o(Vertex, color)  },
	{ Shader::Position, 2, GL_FLOAT, o(Vertex, pos)    },
	{ Shader::TexCoord, 2, GL_FLOAT, o(Vertex, texPos) }
};

#define DEF_TRAITS(VertType) \
	template<> \
	const VertexAttribute *VertexTraits<VertType>::attr = VertType##Attribs; \
	template<> \
	const GLsizei VertexTraits<VertType>::attrCount = ARRAY_SIZE(VertType##Attribs)

DEF_TRAITS(SVertex);
DEF_TRAITS(CVertex);
DEF_TRAITS(Vertex);
