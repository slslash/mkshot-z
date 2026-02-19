/*
** filesystemImpl.h
** Player
**
** This file is part of mkxp-z, further modified for mkshot-z.
**
** mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2013 - 2023 mkxp-z contributors <https://github.com/mkxp-z/mkxp-z/graphs/contributors>
**
** Created by ゾロアーク on 11/21/20.
*/

#ifndef filesystemImpl_h
#define filesystemImpl_h

#include <string>
#include <SDL3/SDL_video.h>

namespace filesystemImpl {
bool fileExists(const char *path);

std::string contentsOfFileAsString(const char *path);

bool setCurrentDirectory(const char *path);
    
std::string getCurrentDirectory();
    
std::string normalizePath(const char *path, bool preferred, bool absolute);

std::string getDefaultGameRoot();

#ifdef MKXPZ_BUILD_XCODE
std::string getPathForAsset(const char *baseName, const char *ext);
std::string contentsOfAssetAsString(const char *baseName, const char *ext);

std::string getResourcePath();

std::string selectPath(SDL_Window *win, const char *msg, const char *prompt);
#endif

};
#endif /* filesystemImpl_h */
