/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef QUADARRAY_H
#define QUADARRAY_H

#include "vertex.h"
#include "gl-util.h"
#include "gl-meta.h"
#include "sharedstate.h"
#include "global-ibo.h"
#include "shader.h"

#include <vector>
#include <stdint.h>

template<class VertexType>
struct QuadArray
{
	std::vector<VertexType> vertices;

	VBO::ID vbo;
	GLMeta::VAO vao;

	size_t quadCount;
	GLsizeiptr vboSize;

	QuadArray()
	    : quadCount(0),
	      vboSize(-1)
	{
		vbo = VBO::gen();

		GLMeta::vaoFillInVertexData<VertexType>(vao);
		vao.vbo = vbo;
		vao.ibo = shState->globalIBO().ibo;

		GLMeta::vaoInit(vao);
	}

	~QuadArray()
	{
		GLMeta::vaoFini(vao);
		VBO::del(vbo);
	}

	void resize(size_t size)
	{
		vertices.resize(size * 4);
		quadCount = size;
	}

	void clear()
	{
		vertices.clear();
		quadCount = 0;
	}

	/* This needs to be called after the final 'append()' call
	 * and previous to the first 'draw()' call. */
	void commit()
	{
		VBO::bind(vbo);

		GLsizeiptr size = vertices.size() * sizeof(VertexType);

		if (size > vboSize)
		{
			/* New data exceeds already allocated size.
			 * Reallocate VBO. */
			VBO::uploadData(size, dataPtr(vertices), GL_DYNAMIC_DRAW);
			vboSize = size;

			shState->ensureQuadIBO(quadCount);
		}
		else
		{
			/* New data fits in allocated size */
			VBO::uploadSubData(0, size, dataPtr(vertices));
		}

		VBO::unbind();
	}

	void draw(size_t offset, size_t count)
	{
		GLMeta::vaoBind(vao);

		const char *_offset = (const char*) 0 + offset * 6 * sizeof(index_t);
		gl.DrawElements(GL_TRIANGLES, count * 6, _GL_INDEX_TYPE, _offset);

		GLMeta::vaoUnbind(vao);
	}

	void draw()
	{
		draw(0, quadCount);
	}

	size_t count() const
	{
		return quadCount;
	}
};

typedef QuadArray<Vertex> ColorQuadArray;
typedef QuadArray<SVertex> SimpleQuadArray;

#endif // QUADARRAY_H
