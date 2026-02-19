/*
** gnome-fun.cpp
**
** This file is part of ModShot-mkxp-z, further modified for mkshot-z.
**
** ModShot-mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2024 hat_kid <https://github.com/thehatkid>
*/

#include "gnome-fun.h"
#include "debugwriter.h"

#include <SDL3/SDL_loadso.h>

#ifdef __LINUX__
#define GTK2_LIBNAME "libgtk-x11-2.0.so.0"
#define GTK3_LIBNAME "libgtk-3.so.0"
#define GDK2_LIBNAME "libgdk-x11-2.0.so.0"
#define GDK3_LIBNAME "libgdk-3.so.0"
#define GIO2_LIBNAME "libgio-2.0.so.0"
#else
#error "This code is currently only for Linux platforms." // TODO: how about no
#endif

struct GnomeFunctions dynGnome;
struct GioFunctions dynGio;
struct GdkFunctions dynGdk;

#define GNOME_FUNC(name, type) \
	dynGnome.name = (type)SDL_LoadFunction(so, #name); \
	if (dynGnome.name == NULL) { \
		Debug() << "[gnome-fun] Unable to load GTK+ function:" << SDL_GetError(); \
		fail = true; \
	}

#define GIO_FUNC(name, type) \
	dynGio.name = (type)SDL_LoadFunction(so, #name); \
	if (dynGio.name == NULL) { \
		Debug() << "[gnome-fun] Unable to load Gio function:" << SDL_GetError(); \
		fail = true; \
	}

#define GDK_FUNC(name, type) \
	dynGdk.name = (type)SDL_LoadFunction(so, #name); \
	if (dynGdk.name == NULL) { \
		Debug() << "[gnome-fun] Unable to load GDK function:" << SDL_GetError(); \
		fail = true; \
	}

void initGnomeFunctions()
{
	bool fail = false;

	static void *so;

	// Try to load GTK+ 3
	so = SDL_LoadObject(GTK3_LIBNAME);

	if (so == NULL) {
		Debug() << "[gnome-fun] Couldn't load GTK+ 3 library:" << SDL_GetError();

		// Try to load GTK+ 2
		so = SDL_LoadObject(GTK2_LIBNAME);

		if (so == NULL) {
			Debug() << "[gnome-fun] Couldn't load GTK+ 2 library:" << SDL_GetError();
			fail = true;
		}
	}

	if (!fail) {
		GLIB_FUNCS
		GTK_FUNCS
	}

	if (fail) {
		memset(&dynGnome, 0, sizeof(dynGnome));
		SDL_UnloadObject(so);
		so = nullptr;
	}
}

void initGioFunctions()
{
	bool fail = false;

	static void *so;

	// Try to load Gio 2.0
	so = SDL_LoadObject(GTK3_LIBNAME);

	if (so == NULL) {
		Debug() << "[gnome-fun] Couldn't load Gio 2.0 library:" << SDL_GetError();
		fail = true;
	}

	if (!fail) {
		GIO_FUNCS
	}

	if (fail) {
		memset(&dynGio, 0, sizeof(dynGio));
		SDL_UnloadObject(so);
		so = nullptr;
	}
}

void initGdkFunctions()
{
	bool fail = false;

	static void *so;

	// Try to load GDK 3
	so = SDL_LoadObject(GTK3_LIBNAME);

	if (so == NULL) {
		Debug() << "[gnome-fun] Couldn't load GDK 3 library:" << SDL_GetError();

		// Try to load GDK 2
		so = SDL_LoadObject(GTK2_LIBNAME);

		if (so == NULL) {
			Debug() << "[gnome-fun] Couldn't load GDK 2 library:" << SDL_GetError();
			fail = true;
		}
	}

	if (!fail) {
		GDK_FUNCS
	}

	if (fail) {
		memset(&dynGnome, 0, sizeof(dynGnome));
		SDL_UnloadObject(so);
		so = nullptr;
	}
}
