/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef SPRITE_H
#define SPRITE_H

#include "scene.h"
#include "flashable.h"
#include "disposable.h"
#include "viewport.h"
#include "util.h"

class Bitmap;
struct Color;
struct Tone;
struct Rect;

struct SpritePrivate;

class Sprite : public ViewportElement, public Flashable, public Disposable
{
public:
	Sprite(Viewport *viewport = 0);
	~Sprite();

	int getWidth()  const;
	int getHeight() const;

	void update();

	DECL_ATTR( Bitmap,      Bitmap* )
	DECL_ATTR( SrcRect,     Rect&   )
	DECL_ATTR( X,           int     )
	DECL_ATTR( Y,           int     )
	DECL_ATTR( OX,          int     )
	DECL_ATTR( OY,          int     )
	DECL_ATTR( ZoomX,       float   )
	DECL_ATTR( ZoomY,       float   )
	DECL_ATTR( Angle,       float   )
	DECL_ATTR( Mirror,      bool    )
	DECL_ATTR( BushDepth,   int     )
	DECL_ATTR( BushOpacity, int     )
	DECL_ATTR( Opacity,     int     )
	DECL_ATTR( BlendType,   int     )
	DECL_ATTR( Color,       Color&  )
	DECL_ATTR( Tone,        Tone&   )
    DECL_ATTR( Pattern,     Bitmap* )
    DECL_ATTR( PatternBlendType, int)
    DECL_ATTR( PatternTile, bool    )
    DECL_ATTR( PatternOpacity, int  )
    DECL_ATTR( PatternScrollX, int  )
    DECL_ATTR( PatternScrollY, int  )
    DECL_ATTR( PatternZoomX, float  )
    DECL_ATTR( PatternZoomY, float  )
    DECL_ATTR( Invert,      bool    )
	DECL_ATTR( Obscured,    bool    )
	DECL_ATTR( WaveAmp,     int     )
	DECL_ATTR( WaveLength,  int     )
	DECL_ATTR( WaveSpeed,   int     )
	DECL_ATTR( WavePhase,   float   )

	void initDynAttribs();

private:
	SpritePrivate *p;

	void draw();
	void onGeometryChange(const Scene::Geometry &);

	void releaseResources();
	const char *klassName() const { return "sprite"; }

	ABOUT_TO_ACCESS_DISP
};

#endif // SPRITE_H
