/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef FLASHABLE_H
#define FLASHABLE_H

#include "etc.h"
#include "etc-internal.h"

class Flashable
{
public:
	Flashable()
	    : flashColor(0, 0, 0, 0),
	      flashing(false),
	      emptyFlashFlag(false)
	{}

	virtual ~Flashable() {}

	void flash(const Vec4 *color, int duration)
	{
		if (duration < 1)
			return;

		flashing = true;
		this->duration = duration;
		counter = 0;

		if (!color)
		{
			emptyFlashFlag = true;
			return;
		}

		flashColor = *color;
		flashAlpha = flashColor.w;
	}

	virtual void update()
	{
		if (!flashing)
			return;

		if (++counter > duration)
		{
			/* Flash finished. Cleanup */
			flashColor = Vec4(0, 0, 0, 0);
			flashing = false;
			emptyFlashFlag = false;
			return;
		}

		/* No need to update flash color on empty flash */
		if (emptyFlashFlag)
			return;

		float prog = (float) counter / duration;
		flashColor.w = flashAlpha * (1 - prog);
	}

protected:
	Vec4 flashColor;
	bool flashing;
	bool emptyFlashFlag;
private:
	float flashAlpha;
	int duration;
	int counter;
};

#endif // FLASHABLE_H
