/*
** keybindings.h
**
** This file is part of mkxp, further modified for mkshot-z.
**
** mkxp is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include "input.h"

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_gamepad.h>
#include <stdint.h>
#include <assert.h>
#include <vector>

enum AxisDir
{
	Negative,
	Positive
};

enum SourceType
{
	Invalid,
	Key,
    CButton,
    CAxis
};

struct SourceDesc
{
	SourceType type;

	union Data
	{
		/* Keyboard scancode */
		SDL_Scancode scan;
		/* Joystick button index */
		SDL_GameControllerButton cb;
		struct
		{
			/* Joystick axis index */
			SDL_GameControllerAxis axis;
			/* Joystick axis direction */
			AxisDir dir;
		} ca;
	} d;

	bool operator==(const SourceDesc &o) const
	{
		if (type != o.type)
			return false;

		switch (type)
		{
		case Invalid:
			return true;
		case Key:
			return d.scan == o.d.scan;
        case CButton:
            return d.cb == o.d.cb;
		case CAxis:
			return (d.ca.axis == o.d.ca.axis) && (d.ca.dir == o.d.ca.dir);
		default:
			assert(!"unreachable");
			return false;
		}
	}

	bool operator!=(const SourceDesc &o) const
	{
		return !(*this == o);
	}
};

#define JAXIS_THRESHOLD 0x4000

struct BindingDesc
{
	SourceDesc src;
	Input::ButtonCode target;
};

typedef std::vector<BindingDesc> BDescVec;
struct Config;

BDescVec genDefaultBindings(const Config &conf);

void storeBindings(const BDescVec &d, const Config &conf);
BDescVec loadBindings(const Config &conf);

#endif // KEYBINDINGS_H
