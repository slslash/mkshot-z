/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef TEXPOOL_H
#define TEXPOOL_H

#include "gl-util.h"

struct TexPoolPrivate;

class TexPool
{
public:
	TexPool(uint32_t maxMemSize = 20000000 /* 20 MB */);
	~TexPool();

	TEXFBO request(int width, int height);
	void release(TEXFBO &obj);

	void disable();

private:
	TexPoolPrivate *p;
};

#endif // TEXPOOL_H
