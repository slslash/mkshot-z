/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef GLOBALIBO_H
#define GLOBALIBO_H

#include "gl-util.h"

#include <vector>
#include <limits>
#include <assert.h>
#include <stdint.h>

typedef uint16_t index_t;
#define INDEX_T_MAX std::numeric_limits<index_t>::max()
#define _GL_INDEX_TYPE GL_UNSIGNED_SHORT

struct GlobalIBO
{
	IBO::ID ibo;
	std::vector<index_t> buffer;

	GlobalIBO()
	{
		ibo = IBO::gen();
	}

	~GlobalIBO()
	{
		IBO::del(ibo);
	}

	void ensureSize(size_t quadCount)
	{
		assert(quadCount*6 < INDEX_T_MAX);

		if (buffer.size() >= quadCount*6)
			return;

		size_t startInd = buffer.size() / 6;
		buffer.reserve(quadCount*6);

		for (size_t i = startInd; i < quadCount; ++i)
		{
			static const index_t indTemp[] = { 0, 1, 2, 2, 3, 0 };

			for (size_t j = 0; j < 6; ++j)
				buffer.push_back(i * 4 + indTemp[j]);
		}

		IBO::bind(ibo);
		IBO::uploadData(buffer.size() * sizeof(index_t), dataPtr(buffer));
		IBO::unbind();
	}
};

#endif // GLOBALIBO_H
