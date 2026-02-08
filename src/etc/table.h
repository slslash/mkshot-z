/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef TABLE_H
#define TABLE_H

#include "serializable.h"

#include <stdint.h>
#include "sigslot/signal.hpp"
#include <vector>

class Table : public Serializable
{
public:
	Table(int x, int y = 1, int z = 1);
	/* Clone constructor */
	Table(const Table &other);
	virtual ~Table() {}

	int xSize() const { return xs; }
	int ySize() const { return ys; }
	int zSize() const { return zs; }

	int16_t get(int x, int y = 0, int z = 0) const;
	void set(int16_t value, int x, int y = 0, int z = 0);

	void resize(int x, int y, int z);
	void resize(int x, int y);
	void resize(int x);

	int serialSize() const;
	void serialize(char *buffer) const;
	static Table *deserialize(const char *data, int len);

	/* <internal */
	inline int16_t &at(int x, int y = 0, int z = 0)
	{
		return data[xs*ys*z + xs*y + x];
	}

	inline const int16_t &at(int x, int y = 0, int z = 0) const
	{
		return data[xs*ys*z + xs*y + x];
	}

    sigslot::signal<> modified;

private:
	int xs, ys, zs;
	std::vector<int16_t> data;
};

#endif // TABLE_H
