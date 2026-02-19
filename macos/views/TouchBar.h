/*
** TouchBar.h
**
** This file is part of mkxp-z, further modified for mkshot-z.
**
** mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2013 - 2023 mkxp-z contributors <https://github.com/mkxp-z/mkxp-z/graphs/contributors>
**
** Created by ゾロア on 1/14/22.
*/

#ifndef MKXPZ_TOUCHBAR_H
#define MKXPZ_TOUCHBAR_H

#include <stdio.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include "config.h"

#ifdef __OBJC__
API_AVAILABLE(macos(10.12.2))
@interface MKXPZTouchBar : NSTouchBar <NSTouchBarDelegate>
+(MKXPZTouchBar *)sharedTouchBar;
@end
#endif

void initTouchBar(SDL_Window *win, Config &conf);
void updateTouchBarFPSDisplay(uint32_t value);

#endif // MKXPZ_TOUCHBAR_H
