/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef DISPOSABLE_H
#define DISPOSABLE_H

#include "intrulist.h"
#include "exception.h"
#include "sharedstate.h"
#include "graphics.h"

#include <assert.h>
#include "sigslot/signal.hpp"

class Disposable
{
public:
	Disposable()
	    : disposed(false),
	      link(this)
	{
		shState->graphics().addDisposable(this);
	}

	virtual ~Disposable()
	{
		shState->graphics().remDisposable(this);
	}

	void dispose()
	{
		if (disposed)
			return;

		releaseResources();
		disposed = true;
		wasDisposed();
	}

	bool isDisposed() const
	{
		return disposed;
	}

    sigslot::signal<> wasDisposed;

protected:
	void guardDisposed() const
	{
		if (isDisposed())
			throw Exception(Exception::RGSSError,
		                    "disposed %s", klassName());
	}

private:
	virtual void releaseResources() = 0;
	virtual const char *klassName() const = 0;

	friend class Graphics;

	bool disposed;
	IntruListLink<Disposable> link;
};

template<class C>
inline bool
nullOrDisposed(const C *d)
{
	if (!d)
		return true;

	if (d->isDisposed())
		return true;

	return false;
}

#endif // DISPOSABLE_H
