// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ONESHOT_WALLPAPER_H
#define ONESHOT_WALLPAPER_H

struct WallpaperPrivate;

class Wallpaper
{
private:
	WallpaperPrivate *p;

	void cache();

public:
	Wallpaper();
	~Wallpaper();

	void set(const char *name, int color);
	void reset();
};

#endif // ONESHOT_WALLPAPER_H
