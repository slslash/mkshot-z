/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef TILEMAPVX_H
#define TILEMAPVX_H

#include "disposable.h"
#include "util.h"

class Viewport;
class Bitmap;
class Table;

struct TilemapVXPrivate;

class TilemapVX : public Disposable
{
public:
	class BitmapArray
	{
	public:
		void set(int i, Bitmap *bitmap);
		Bitmap *get(int i) const;

	private:
		BitmapArray() {}
		~BitmapArray() {}

		TilemapVXPrivate *p;
		friend class TilemapVX;
		friend struct TilemapVXPrivate;
	};

	TilemapVX(Viewport *viewport = 0);
	~TilemapVX();

	void update();

	BitmapArray &getBitmapArray();

	DECL_ATTR( Viewport,   Viewport* )
	DECL_ATTR( MapData,    Table*    )
	DECL_ATTR( FlashData,  Table*    )
	DECL_ATTR( Flags,      Table*    )
	DECL_ATTR( Visible,    bool      )
	DECL_ATTR( OX,         int       )
	DECL_ATTR( OY,         int       )

private:
	TilemapVXPrivate *p;
	BitmapArray bmProxy;

	void releaseResources();
	const char *klassName() const { return "tilemap"; }
};

#endif // TILEMAPVX_H
