/*
** filesystem.h
**
** This file is part of mkxp, further modified for mkshot-z.
**
** mkxp is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <SDL3/SDL_iostream.h>
#include <string>

#include "filesystemImpl.h"

namespace mkxp_fs = filesystemImpl;

struct FileSystemPrivate;
class SharedFontState;

class FileSystem
{
public:
	FileSystem(const char *argv0,
	           bool allowSymlinks);
	~FileSystem();

	void addPath(const char *path, const char *mountpoint = 0, bool reload = false);
    void removePath(const char *path, bool reload = false);

	/* Call these after the last 'addPath()' */
	void createPathCache();
    
    void reloadPathCache();

	/* Scans "Fonts/" and creates inventory of
	 * available font assets */
	void initFontSets(SharedFontState &sfs);

	struct OpenHandler
	{
		/* Try to read and interpret data provided from ops.
		 * If data cannot be parsed, return false, otherwise true.
		 * Can be called multiple times until a parseable file is found.
		 * It's the handler's responsibility to close every passed
		 * ops structure, even when data could not be parsed.
		 * After this function returns, ops becomes invalid, so don't take
		 * references to it. Instead, copy the structure without closing
		 * if you need to further read from it later. */
		virtual bool tryRead(SDL_RWops &ops, const char *ext) = 0;
	};

	void openRead(OpenHandler &handler,
	              const char *filename);

	/* Circumvents extension supplementing */
	void openReadRaw(SDL_RWops &ops,
	                 const char *filename,
	                 bool freeOnClose = false);

	std::string normalize(const char *pathname, bool preferred, bool absolute);

	/* Does not perform extension supplementing */
	bool exists(const char *filename);

	const char *desensitize(const char *filename);

private:
	FileSystemPrivate *p;
};

extern const Uint32 SDL_RWOPS_PHYSFS;

#endif // FILESYSTEM_H
