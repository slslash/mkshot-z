/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef SOUNDEMITTER_H
#define SOUNDEMITTER_H

#include "intrulist.h"
#include "al-util.h"
#include "boost-hash.h"

#include <string>
#include <vector>

struct SoundBuffer;
struct Config;

struct SoundEmitter
{
	typedef BoostHash<std::string, SoundBuffer*> BufferHash;

	IntruList<SoundBuffer> buffers;
	BufferHash bufferHash;

	/* Byte count sum of all cached / playing buffers */
	uint32_t bufferBytes;

	const size_t srcCount;
	std::vector<AL::Source::ID> alSrcs;
	std::vector<SoundBuffer*> atchBufs;

	/* Indices of sources, sorted by priority (lowest first) */
	std::vector<size_t> srcPrio;

	SoundEmitter(const Config &conf);
	~SoundEmitter();

	void play(const std::string &filename,
	          int volume,
	          int pitch);

	void stop();

private:
	SoundBuffer *allocateBuffer(const std::string &filename);
};

#endif // SOUNDEMITTER_H
