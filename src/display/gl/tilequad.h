/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef TILEQUAD_H
#define TILEQUAD_H

#include "etc-internal.h"

/* Tiled Quads
 *
 * These functions enable drawing a tiled subrectangle
 * of a texture,
 * but no advanced stuff like rotation, scaling etc.
 */

#include "quadarray.h"

namespace TileQuads
{
	/* Calculate needed quad counts */
	int oneDimCount(int tileDimension,
	                int destDimension);
	int twoDimCount(int tileW, int tileH,
	                int destW, int destH);

	/* Build tiling quads */
	int buildH(const IntRect &sourceRect,
	           int width, int x, int y,
	           Vertex *verts);

	int buildV(const IntRect &sourceRect,
	           int height, int ox, int oy,
	           Vertex *verts);

	int build(const IntRect &sourceRect,
	          const IntRect &destRect,
	          Vertex *verts);

	/* Build a quad "frame" (see Window cursor_rect) */
	int buildFrame(const IntRect &rect,
	               Vertex vert[36]);

	int buildFrameSource(const IntRect &rect,
	                     Vertex vert[36]);
}

#endif // TILEQUAD_H
