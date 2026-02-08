/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#ifndef ALDATASOURCE_H
#define ALDATASOURCE_H

#include "al-util.h"

struct ALDataSource
{
	enum Status
	{
		NoError,
		EndOfStream,
		WrapAround,
		Error
	};

	virtual ~ALDataSource() {}

	/* Read/process next chunk of data, and attach it
	 * to provided AL buffer */
	virtual Status fillBuffer(AL::Buffer::ID alBuffer) = 0;

	virtual int sampleRate() = 0;

	/* If the source doesn't support seeking, it will
	 * reset back to the beginning */
	virtual void seekToOffset(float seconds) = 0;

	/* The frame count right after wrap around */
	virtual uint32_t loopStartFrames() = 0;

	/* Returns false if not supported */
	virtual bool setPitch(float value) = 0;
};

ALDataSource *createSDLSource(SDL_RWops &ops,
                              const char *extension,
			                  uint32_t maxBufSize,
			                  bool looped,
			                  int fallbackMode);

ALDataSource *createVorbisSource(SDL_RWops &ops,
                                 bool looped);

ALDataSource *createMidiSource(SDL_RWops &ops,
                               bool looped);

#endif // ALDATASOURCE_H
