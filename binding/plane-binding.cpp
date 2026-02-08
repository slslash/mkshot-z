/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "binding-types.h"
#include "binding-util.h"
#include "disposable-binding.h"
#include "plane.h"
#include "viewportelement-binding.h"

#if RAPI_FULL > 187
DEF_TYPE(Plane);
#else
DEF_ALLOCFUNC(Plane);
#endif

RB_METHOD(planeInitialize)
{
	Plane *p = viewportElementInitialize<Plane>(argc, argv, self);

	setPrivateData(self, p);

	GFX_LOCK;
	p->initDynAttribs();
	wrapProperty(self, &p->getColor(), "color", ColorType);
	wrapProperty(self, &p->getTone(), "tone", ToneType);
	wrapProperty(self, &p->getSrcRect(), "src_rect", RectType);
	GFX_UNLOCK;

	return self;
}

DEF_GFX_PROP_OBJ_REF(Plane, Bitmap, Bitmap, "bitmap")
DEF_GFX_PROP_OBJ_VAL(Plane, Color, Color, "color")
DEF_GFX_PROP_OBJ_VAL(Plane, Tone, Tone, "tone")
DEF_GFX_PROP_OBJ_VAL(Plane, Rect, SrcRect, "src_rect")

DEF_GFX_PROP_I(Plane, OX)
DEF_GFX_PROP_I(Plane, OY)
DEF_GFX_PROP_I(Plane, Opacity)
DEF_GFX_PROP_I(Plane, BlendType)

DEF_GFX_PROP_F(Plane, ZoomX)
DEF_GFX_PROP_F(Plane, ZoomY)

void planeBindingInit() {
  VALUE klass = rb_define_class("Plane", rb_cObject);
#if RAPI_FULL > 187
  rb_define_alloc_func(klass, classAllocate<&PlaneType>);
#else
  rb_define_alloc_func(klass, PlaneAllocate);
#endif

  disposableBindingInit<Plane>(klass);
  viewportElementBindingInit<Plane>(klass);

  _rb_define_method(klass, "initialize", planeInitialize);

  INIT_PROP_BIND(Plane, Bitmap, "bitmap");
  INIT_PROP_BIND(Plane, OX, "ox");
  INIT_PROP_BIND(Plane, OY, "oy");
  INIT_PROP_BIND(Plane, ZoomX, "zoom_x");
  INIT_PROP_BIND(Plane, ZoomY, "zoom_y");
  INIT_PROP_BIND(Plane, Opacity, "opacity");
  INIT_PROP_BIND(Plane, BlendType, "blend_type");
  INIT_PROP_BIND(Plane, Color, "color");
  INIT_PROP_BIND(Plane, Tone, "tone");
  INIT_PROP_BIND(Plane, SrcRect, "src_rect");
}
