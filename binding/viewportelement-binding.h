/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef VIEWPORTELEMENTBINDING_H
#define VIEWPORTELEMENTBINDING_H

#include "viewport.h"
#include "sharedstate.h"
#include "binding-util.h"
#include "binding-types.h"
#include "graphics.h"

#include "sceneelement-binding.h"
#include "disposable-binding.h"

template<class C>
RB_METHOD(viewportElementGetViewport)
{
	RB_UNUSED_PARAM;

	checkDisposed<C>(self);

	return rb_iv_get(self, "viewport");
}

template<class C>
RB_METHOD(viewportElementSetViewport)
{
	RB_UNUSED_PARAM;

	ViewportElement *ve = getPrivateData<C>(self);

	VALUE viewportObj = Qnil;
	Viewport *viewport = 0;

	rb_get_args(argc, argv, "o", &viewportObj RB_ARG_END);

	if (!NIL_P(viewportObj))
		viewport = getPrivateDataCheck<Viewport>(viewportObj, ViewportType);

	GFX_GUARD_EXC( ve->setViewport(viewport); );

	rb_iv_set(self, "viewport", viewportObj);

	return viewportObj;
}

template<class C>
static C *
viewportElementInitialize(int argc, VALUE *argv, VALUE self)
{
	/* Get parameters */
	VALUE viewportObj = Qnil;
	Viewport *viewport = 0;

	rb_get_args(argc, argv, "|o", &viewportObj RB_ARG_END);

	if (!NIL_P(viewportObj))
	{
		viewport = getPrivateDataCheck<Viewport>(viewportObj, ViewportType);
	}

    GFX_LOCK;
	/* Construct object */
	C *ve = new C(viewport);

    
	/* Set property objects */
	rb_iv_set(self, "viewport", viewportObj);
    GFX_UNLOCK;
	return ve;
}

template<class C>
void
viewportElementBindingInit(VALUE klass)
{
	sceneElementBindingInit<C>(klass);

	_rb_define_method(klass, "viewport", viewportElementGetViewport<C>);

    //if (rgssVer >= 2)
	//{
	_rb_define_method(klass, "viewport=", viewportElementSetViewport<C>);
	//}
}

#endif // VIEWPORTELEMENTBINDING_H
