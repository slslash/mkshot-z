/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef BINDINGTYPES_H
#define BINDINGTYPES_H

#include "binding-util.h"

#if RAPI_FULL > 187
DECL_TYPE(Table);
DECL_TYPE(Rect);
DECL_TYPE(Color);
DECL_TYPE(Tone);
DECL_TYPE(Font);

DECL_TYPE(Bitmap);
DECL_TYPE(Sprite);
DECL_TYPE(Plane);
DECL_TYPE(Viewport);
DECL_TYPE(Tilemap);
DECL_TYPE(Window);

DECL_TYPE(MiniFFI);

#else
#define TableType "Table"
#define RectType "Rect"
#define ColorType "Color"
#define ToneType "Tone"
#define FontType "Font"

#define BitmapType "Bitmap"
#define SpriteType "Sprite"
#define PlaneType "Plane"
#define ViewportType "Viewport"
#define TilemapType "Tilemap"
#define WindowType "Window"

#define MiniFFIType "MiniFFI"
#endif

#endif // BINDINGTYPES_H
