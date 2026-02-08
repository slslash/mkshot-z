/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef TILEATLASVX_H
#define TILEATLASVX_H

#include <stdlib.h>

struct FloatRect;
struct TEXFBO;
class Bitmap;
class Table;

#define ATLASVX_W 1024
#define ATLASVX_H 2048

/* Bitmap indices */
enum
{
	BM_A1 = 0,
	BM_A2 = 1,
	BM_A3 = 2,
	BM_A4 = 3,
	BM_A5 = 4,
	BM_B  = 5,
	BM_C  = 6,
	BM_D  = 7,
	BM_E  = 8,

	BM_COUNT
};

namespace TileAtlasVX
{
struct Reader
{
	virtual void onQuads(const FloatRect *t, const FloatRect *p,
	                     size_t n, bool overPlayer) = 0;
};

void build(TEXFBO &tf, Bitmap *bitmaps[BM_COUNT]);

void readTiles(Reader &reader, const Table &data,
               const Table *flags, int ox, int oy, int w, int h);
}

#endif // TILEATLASVX_H
