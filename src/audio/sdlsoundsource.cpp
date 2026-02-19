/*
** sdlsoundsource.cpp
**
** This file is part of mkxp, further modified for mkshot-z.
**
** mkxp is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "aldatasource.h"
#include "exception.h"

#include <SDL3_sound/SDL_sound.h>

struct SDLSoundSource : ALDataSource
{
	Sound_Sample *sample;
	SDL_RWops &srcOps;
	uint8_t sampleSize;
	bool looped;

	ALenum alFormat;
	ALsizei alFreq;

	SDLSoundSource(SDL_RWops &ops,
	               const char *extension,
	               uint32_t maxBufSize,
	               bool looped,
	               int fallbackMode)
	    : srcOps(ops),
	      looped(looped)
	{
		if (fallbackMode == 0)
		{
			sample = Sound_NewSample(&srcOps, extension, 0, maxBufSize);
		}
		else
		{
			// We're here because a previous attempt resulted in S32 format.

			Sound_AudioInfo desired;
			SDL_memset(&desired, '\0', sizeof (Sound_AudioInfo));
			desired.format = AUDIO_F32SYS;

			sample = Sound_NewSample(&srcOps, extension, &desired, maxBufSize);
		}

		if (!sample)
		{
			SDL_RWclose(&ops);
			throw Exception(Exception::SDLError, "SDL_sound: %s", Sound_GetError());
		}

		if (fallbackMode == 0)
		{
			bool validFormat = true;

			switch (sample->actual.format)
			{
			// OpenAL Soft doesn't support S32 formats.
			// https://github.com/kcat/openal-soft/issues/934
			case AUDIO_S32LSB :
			case AUDIO_S32MSB :
				validFormat = false;
			}

			if (!validFormat)
			{
				// Unfortunately there's no way to change the desired format of a sample.
				// https://github.com/icculus/SDL_sound/issues/91
				// So we just have to close the sample (which closes the file too),
				// and retry with a new desired format.
				Sound_FreeSample(sample);
				throw Exception(Exception::SDLError, "SDL_sound: format not supported by OpenAL: %d", sample->actual.format);
			}
		}

		sampleSize = formatSampleSize(sample->actual.format);

		alFormat = chooseALFormat(sampleSize, sample->actual.channels);
		alFreq = sample->actual.rate;
	}

	~SDLSoundSource()
	{
		/* This also closes 'srcOps' */
		Sound_FreeSample(sample);
	}

	Status fillBuffer(AL::Buffer::ID alBuffer)
	{
		uint32_t decoded = Sound_Decode(sample);

		if (sample->flags & SOUND_SAMPLEFLAG_EAGAIN)
		{
			/* Try to decode one more time on EAGAIN */
			decoded = Sound_Decode(sample);

			/* Give up */
			if (sample->flags & SOUND_SAMPLEFLAG_EAGAIN)
				return ALDataSource::Error;
		}

		if (sample->flags & SOUND_SAMPLEFLAG_ERROR)
			return ALDataSource::Error;

		AL::Buffer::uploadData(alBuffer, alFormat, sample->buffer, decoded, alFreq);

		if (sample->flags & SOUND_SAMPLEFLAG_EOF)
		{
			if (looped)
			{
				Sound_Rewind(sample);
				return ALDataSource::WrapAround;
			}
			else
			{
				return ALDataSource::EndOfStream;
			}
		}

		return ALDataSource::NoError;
	}

	int sampleRate()
	{
		return sample->actual.rate;
	}

	void seekToOffset(float seconds)
	{
		if (seconds <= 0)
			Sound_Rewind(sample);
		else
			Sound_Seek(sample, static_cast<uint32_t>(seconds * 1000));
	}

	uint32_t loopStartFrames()
	{
		/* Loops from the beginning of the file */
		return 0;
	}

	bool setPitch(float)
	{
		return false;
	}
};

ALDataSource *createSDLSource(SDL_RWops &ops,
                              const char *extension,
			                  uint32_t maxBufSize,
			                  bool looped,
			                  int fallbackMode)
{
	return new SDLSoundSource(ops, extension, maxBufSize, looped, fallbackMode);
}
