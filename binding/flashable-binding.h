/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef FLASHABLEBINDING_H
#define FLASHABLEBINDING_H

#include "flashable.h"
#include "binding-util.h"
#include "binding-types.h"

template<class C>
RB_METHOD(flashableFlash)
{
	Flashable *f = getPrivateData<C>(self);

	VALUE colorObj;
	int duration;

	Color *color;

	rb_get_args(argc, argv, "oi", &colorObj, &duration RB_ARG_END);

	if (NIL_P(colorObj))
	{
		f->flash(0, duration);
		return Qnil;
	}

	color = getPrivateDataCheck<Color>(colorObj, ColorType);

	f->flash(&color->norm, duration);

	return Qnil;
}

template<class C>
RB_METHOD(flashableUpdate)
{
	RB_UNUSED_PARAM;

	Flashable *f = getPrivateData<C>(self);

	f->update();

	return Qnil;
}

template<class C>
static void flashableBindingInit(VALUE klass)
{
	_rb_define_method(klass, "flash", flashableFlash<C>);
	_rb_define_method(klass, "update", flashableUpdate<C>);
}

#endif // FLASHABLEBINDING_H
