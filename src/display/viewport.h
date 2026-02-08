/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "scene.h"
#include "flashable.h"
#include "disposable.h"
#include "util.h"

struct ViewportPrivate;

class Viewport : public Scene, public SceneElement, public Flashable, public Disposable
{
public:
	Viewport(int x, int y, int width, int height);
	Viewport(Rect *rect);
	Viewport();
	~Viewport();

	void update();

	DECL_ATTR( Rect,  Rect&  )
	DECL_ATTR( OX,    int    )
	DECL_ATTR( OY,    int    )
	DECL_ATTR( Color, Color& )
	DECL_ATTR( Tone,  Tone&  )

	void initDynAttribs();

private:
	void initViewport(int x, int y, int width, int height);
	void geometryChanged();

	void composite();
	void draw();
	void onGeometryChange(const Geometry &);
	bool isEffectiveViewport(Rect *&, Color *&, Tone *&) const;

	void releaseResources();
	const char *klassName() const { return "viewport"; }

	ABOUT_TO_ACCESS_DISP

	ViewportPrivate *p;
	friend struct ViewportPrivate;

	IntruListLink<Scene> sceneLink;
};

class ViewportElement : public SceneElement
{
public:
	ViewportElement(Viewport *viewport = 0, int z = 0, int spriteY = 0);
	~ViewportElement();

	DECL_ATTR( Viewport,  Viewport* )

protected:
	virtual void onViewportChange() {}

private:
	Viewport *m_viewport;
	sigslot::connection viewportDispCon;
	sigslot::connection viewportElementDispCon;
	void viewportElementDisposal();
};

#endif // VIEWPORT_H
