/*
** xfconf-fun.cpp
**
** This file is part of ModShot-mkxp-z, further modified for mkshot-z.
**
** ModShot-mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2024 hat_kid <https://github.com/thehatkid>
*/

#include "xfconf-fun.h"
#include "debugwriter.h"

#include <vector>

#include <SDL3/SDL_loadso.h>

#ifdef __LINUX__
#define XFCONF_LIBNAME "libxfconf-0.so"
#else
#error "This code is currently only for Linux platforms." // TODO: how about no
#endif

struct XfconfFunctions dynXfconf;

#define XFCONF_FUNC(name, type) \
	dynXfconf.name = (type)SDL_LoadFunction(so, "xfconf_" #name); \
	if (dynXfconf.name == NULL) { \
		Debug() << "[xfconf-fun] Unable to load function:" << SDL_GetError(); \
		fail = true; \
	}

void initXfconfFunctions()
{
	bool fail = false;

	static void *so;

	std::vector<const char *> sonames {
		XFCONF_LIBNAME ".3",
		XFCONF_LIBNAME ".2",
		XFCONF_LIBNAME
	};

	for (const char *name : sonames) {
		so = SDL_LoadObject(name);

		if (so != NULL)
			break;
	}

	if (so == NULL) {
		Debug() << "[xfconf-fun] Couldn't load library:" << SDL_GetError();
		fail = true;
	}

	if (!fail) {
		XFCONF_FUNCS
	}

	if (fail) {
		memset(&dynGio, 0, sizeof(dynGio));
		SDL_UnloadObject(so);
		so = nullptr;
	}
}
