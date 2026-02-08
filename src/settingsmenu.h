/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include <stdint.h>

struct SettingsMenuPrivate;
struct RGSSThreadData;
union SDL_Event;

class SettingsMenu
{
public:
	SettingsMenu(RGSSThreadData &rtData);
	~SettingsMenu();

	/* Returns true if the event was consumed */
	bool onEvent(const SDL_Event &event);
	void raise();
	bool destroyReq() const;

private:
	SettingsMenuPrivate *p;
};

#endif // SETTINGSMENU_H
