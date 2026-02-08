/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef PLANE_H
#define PLANE_H

#include "disposable.h"
#include "viewport.h"

class Bitmap;
struct Color;
struct Tone;

struct PlanePrivate;

class Plane : public ViewportElement, public Disposable
{
public:
	Plane(Viewport *viewport = 0);
	~Plane();

	DECL_ATTR( Bitmap,    Bitmap* )
	DECL_ATTR( OX,        int     )
	DECL_ATTR( OY,        int     )
	DECL_ATTR( ZoomX,     float   )
	DECL_ATTR( ZoomY,     float   )
	DECL_ATTR( Opacity,   int     )
	DECL_ATTR( BlendType, int     )
	DECL_ATTR( Color,     Color&  )
	DECL_ATTR( Tone,      Tone&   )
	DECL_ATTR( SrcRect,   Rect&   )

	void initDynAttribs();

private:
	PlanePrivate *p;

	void draw();
	void onGeometryChange(const Scene::Geometry &);

	void releaseResources();
	const char *klassName() const { return "plane"; }

	ABOUT_TO_ACCESS_DISP
};

#endif // PLANE_H
