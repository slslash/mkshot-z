/*
** audio.cpp
**
** This file is part of mkxp, further modified for mkshot-z.
**
** mkxp is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "audio.h"

#include "audiostream.h"
#include "soundemitter.h"
#include "sharedstate.h"
#include "sharedmidistate.h"
#include "eventthread.h"
#include "sdl-util.h"
#include "exception.h"

#include <string>
#include <vector>

#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>

struct AudioPrivate
{
	std::vector<AudioStream *> bgmTracks;
	AudioStream bgs;
	AudioStream me;

	SoundEmitter se;

	SyncPoint &syncPoint;

	struct
	{
		int bgm = 100;
		int sfx = 100;

		std::vector<int> bgmTracksCurrent;
		int bgsCurrent = 100;
		int meCurrent = 100;
	} volume;

	/* The 'MeWatch' is responsible for detecting
	 * a playing ME, quickly fading out the BGM and
	 * keeping it paused/stopped while the ME plays,
	 * and unpausing/fading the BGM back in again
	 * afterwards */
	enum MeWatchState
	{
		MeNotPlaying,
		BgmFadingOut,
		MePlaying,
		BgmFadingIn
	};

	struct
	{
		SDL_Thread *thread;
		AtomicFlag termReq;
		MeWatchState state;
	} meWatch;

	AudioPrivate(RGSSThreadData &rtData)
	    : bgs(ALStream::Looped, "bgs"),
	      me(ALStream::NotLooped, "me"),
	      se(rtData.config),
	      syncPoint(rtData.syncPoint)
	{
		for (int i = 0; i < rtData.config.BGM.trackCount; i++) {
			std::string id = std::string("bgm" + std::to_string(i));
			bgmTracks.push_back(new AudioStream(ALStream::Looped, id.c_str()));
			volume.bgmTracksCurrent.push_back(100);
		}

		meWatch.state = MeNotPlaying;
		meWatch.thread = createSDLThread
			<AudioPrivate, &AudioPrivate::meWatchFun>(this, "audio_mewatch");
	}

	~AudioPrivate()
	{
		meWatch.termReq.set();
		SDL_WaitThread(meWatch.thread, 0);

		for (AudioStream *track : bgmTracks)
			delete track;
	}

	AudioStream *getTrackByIndex(int index)
	{
		if (index < 0)
			index = 0;

		if (index > (int)(bgmTracks.size()) - 1)
			throw Exception(Exception::MKXPError, "requested BGM track %d out of range (max: %d)", index, bgmTracks.size() - 1);

		return bgmTracks[index];
	}

	void setTrackCurrentVolumeByIndex(int index, int volume)
	{
		if (index < 0)
			index = 0;

		if (index > (int)(bgmTracks.size()) - 1)
			throw Exception(Exception::MKXPError, "requested BGM track %d to set volume out of range (max: %d)", index, bgmTracks.size() - 1);

		this->volume.bgmTracksCurrent[index] = clamp(volume, 0, 100);
	}

	void meWatchFun()
	{
		const float fadeOutStep = 1.f / (200  / AUDIO_SLEEP);
		const float fadeInStep  = 1.f / (1000 / AUDIO_SLEEP);

		while (true)
		{
			syncPoint.passSecondarySync();

			if (meWatch.termReq)
				return;

			switch (meWatch.state)
			{
			case MeNotPlaying:
			{
				me.lockStream();

				if (me.stream.queryState() == ALStream::Playing)
				{
					/* ME playing detected. -> FadeOutBGM */
                    for (auto track : bgmTracks)
                        track->extPaused = true;
                    
					meWatch.state = BgmFadingOut;
				}

				me.unlockStream();

				break;
			}

			case BgmFadingOut :
			{
				me.lockStream();

				if (me.stream.queryState() != ALStream::Playing)
				{
					/* ME has ended while fading OUT BGM. -> FadeInBGM */
					me.unlockStream();
					meWatch.state = BgmFadingIn;

					break;
				}
                
                bool shouldBreak = false;
                
                for (int i = 0; i < (int)(bgmTracks.size()); i++) {
                    AudioStream *track = bgmTracks[i];
                    
                    track->lockStream();
                    
                    float vol = track->getVolume(AudioStream::External);
                    vol -= fadeOutStep;
                    
                    if (vol < 0 || track->stream.queryState() != ALStream::Playing) {
                        /* Either BGM has fully faded out, or stopped midway. -> MePlaying */
                        track->setVolume(AudioStream::External, 0);
                        track->stream.pause();
                        track->unlockStream();
                        
                        // check to see if there are any tracks still playing,
                        // and if the last one was ended this round, this branch should exit
                        std::vector<AudioStream*> playingTracks;
                        for (auto t : bgmTracks)
                            if (t->stream.queryState() == ALStream::Playing)
                                playingTracks.push_back(t);
                        
                        
                        if (playingTracks.size() <= 0 && !shouldBreak) shouldBreak = true;
                        continue;
                    }
                    
                    track->setVolume(AudioStream::External, vol);
                    track->unlockStream();
                    
                }
                if (shouldBreak) {
                    meWatch.state = MePlaying;
                    me.unlockStream();
                    break;
                }
                
				me.unlockStream();

				break;
			}

			case MePlaying :
			{
				me.lockStream();

				if (me.stream.queryState() != ALStream::Playing)
                {
                    /* ME has ended */
                    for (auto track : bgmTracks) {
                        track->lockStream();
                        track->extPaused = false;
                        
                        ALStream::State sState = track->stream.queryState();
                        
                        if (sState == ALStream::Paused) {
                            /* BGM is paused. -> FadeInBGM */
                            track->stream.play();
                            meWatch.state = BgmFadingIn;
                        }
                        else {
                            /* BGM is stopped. -> MeNotPlaying */
                            track->setVolume(AudioStream::External, 1.0f);
                            
                            if (!track->noResumeStop)
                                track->stream.play();
                            
                            meWatch.state = MeNotPlaying;
                        }
                        
                        track->unlockStream();
                    }
				}

                me.unlockStream();

				break;
			}

			case BgmFadingIn :
			{
                for (auto track : bgmTracks)
                    track->lockStream();

				if (bgmTracks[0]->stream.queryState() == ALStream::Stopped)
				{
					/* BGM stopped midway fade in. -> MeNotPlaying */
                    for (auto track : bgmTracks)
                        track->setVolume(AudioStream::External, 1.0f);
					meWatch.state = MeNotPlaying;
                    for (auto track : bgmTracks)
                        track->unlockStream();

					break;
				}

				me.lockStream();

				if (me.stream.queryState() == ALStream::Playing)
				{
					/* ME started playing midway BGM fade in. -> FadeOutBGM */
                    for (auto track : bgmTracks)
                        track->extPaused = true;
					meWatch.state = BgmFadingOut;
					me.unlockStream();
                    for (auto track : bgmTracks)
                        track->unlockStream();

					break;
				}

				float vol = bgmTracks[0]->getVolume(AudioStream::External);
				vol += fadeInStep;

				if (vol >= 1)
				{
					/* BGM fully faded in. -> MeNotPlaying */
					vol = 1.0f;
					meWatch.state = MeNotPlaying;
				}

                for (auto track : bgmTracks)
                    track->setVolume(AudioStream::External, vol);

				me.unlockStream();
                for (auto track : bgmTracks)
                    track->unlockStream();

				break;
			}
			}

			SDL_Delay(AUDIO_SLEEP);
		}
	}
};

Audio::Audio(RGSSThreadData &rtData)
	: p(new AudioPrivate(rtData))
{}


void Audio::bgmPlay(const char *filename,
                    int volume,
                    int pitch,
                    float pos,
                    int track)
{
    int vol = clamp(volume, 0, 100);
    if (track == -127) {
        for (int i = 0; i < (int)p->bgmTracks.size(); i++) {
            if (i == 0) {
                continue;
            }
            p->bgmTracks[i]->stop();
        }
        
        track = 0;
    }
    p->setTrackCurrentVolumeByIndex(track, vol);
    p->getTrackByIndex(track)->play(filename, (vol * p->volume.bgm) / 100, pitch, pos);
}

void Audio::bgmStop(int track)
{
    if (track == -127) {
        for (auto track : p->bgmTracks)
            track->stop();
        
        return;
    }
    
    p->getTrackByIndex(track)->stop();
}

void Audio::bgmFade(int time, int track)
{
    if (track == -127) {
        for (auto track : p->bgmTracks)
            track->fadeOut(time);
        
        return;
    }
    
    p->getTrackByIndex(track)->fadeOut(time);
}

int Audio::bgmGetVolume(int track)
{
    if (track == -127)
        return p->bgmTracks[0]->getVolume(AudioStream::BaseRatio) * 100;
    
    return p->getTrackByIndex(track)->getVolume(AudioStream::Base) * 100;
}

void Audio::bgmSetVolume(int volume, int track)
{
    float vol = clamp(volume, 0, 100) / 100.0;
    if (track == -127) {
        for (auto track : p->bgmTracks)
            track->setVolume(AudioStream::BaseRatio, vol);
        
        return;
    }
    p->setTrackCurrentVolumeByIndex(track, clamp(volume, 0, 100));
    p->getTrackByIndex(track)->setVolume(AudioStream::Base, ((float)(vol * p->volume.bgm)) / 100.0f);
}


void Audio::bgsPlay(const char *filename,
                    int volume,
                    int pitch,
                    float pos)
{
	int vol = clamp(volume, 0, 100);
	p->volume.bgsCurrent = vol;
	p->bgs.play(filename, (vol * p->volume.sfx) / 100, pitch, pos);
}

void Audio::bgsStop()
{
	p->bgs.stop();
}

void Audio::bgsFade(int time)
{
	p->bgs.fadeOut(time);
}


void Audio::mePlay(const char *filename,
                   int volume,
                   int pitch)
{
	int vol = clamp(volume, 0, 100);
	p->volume.meCurrent = vol;
	p->me.play(filename, (vol * p->volume.bgm) / 100, pitch);
}

void Audio::meStop()
{
	p->me.stop();
}

void Audio::meFade(int time)
{
	p->me.fadeOut(time);
}


void Audio::sePlay(const char *filename,
                   int volume,
                   int pitch)
{
	int vol = clamp(volume, 0, 100);
	p->se.play(filename, (vol * p->volume.sfx) / 100, pitch);
}

void Audio::seStop()
{
	p->se.stop();
}

void Audio::setupMidi()
{
	shState->midiState().initIfNeeded(shState->config());
}

float Audio::bgmPos(int track)
{
	return p->getTrackByIndex(track)->playingOffset();
}

float Audio::bgsPos()
{
	return p->bgs.playingOffset();
}

void Audio::reset()
{
    for (auto track : p->bgmTracks) {
    	track->stop();
    }

	p->bgs.stop();
	p->me.stop();
	p->se.stop();
}

int Audio::getGlobalBGMVolume() const
{
	return p->volume.bgm;
}

int Audio::getGlobalSFXVolume() const
{
	return p->volume.sfx;
}

void Audio::setGlobalBGMVolume(int value)
{
	p->volume.bgm = clamp(value, 0, 100);

	int i = 0;
	for (AudioStream *track : p->bgmTracks) {
		track->lockStream();
		track->setVolume(AudioStream::Base, ((float)(p->volume.bgm * p->volume.bgmTracksCurrent[i])) / 10000.0f);
		track->unlockStream();
		i++;
	}

	p->me.lockStream();
	p->me.setVolume(AudioStream::Base, ((float)(p->volume.bgm * p->volume.meCurrent)) / 10000.0f);
	p->me.unlockStream();
}

void Audio::setGlobalSFXVolume(int value)
{
	p->volume.sfx = clamp(value, 0, 100);

	p->bgs.lockStream();
	p->bgs.setVolume(AudioStream::Base, ((float)(p->volume.sfx * p->volume.bgsCurrent)) / 10000.0f);
	p->bgs.unlockStream();
}

Audio::~Audio() { delete p; }
