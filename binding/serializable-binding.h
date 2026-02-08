/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef SERIALIZABLEBINDING_H
#define SERIALIZABLEBINDING_H

#include "serializable.h"
#include "binding-util.h"
#include "exception.h"

template<class C>
static VALUE
serializableDump(int, VALUE *, VALUE self)
{
	Serializable *s = getPrivateData<C>(self);

	int dataSize = s->serialSize();

	VALUE data = rb_str_new(0, dataSize);

	GUARD_EXC( s->serialize(RSTRING_PTR(data)); );

	return data;
}

template<class C>
void
serializableBindingInit(VALUE klass)
{
	_rb_define_method(klass, "_dump", serializableDump<C>);
}

#endif // SERIALIZABLEBINDING_H
