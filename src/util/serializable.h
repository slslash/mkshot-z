/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

struct Serializable
{
	virtual int serialSize() const = 0;
	virtual void serialize(char *buffer) const = 0;
};

template<class C>
C *deserialize(const char *data)
{
	return C::deserialize(data);
}

#endif // SERIALIZABLE_H
