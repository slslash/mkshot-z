/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2013 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "audio.h"
#include "sharedstate.h"
#include "binding-util.h"
#include "exception.h"

#define DEF_PLAY_STOP_POS(entity) \
	RB_METHOD(audio_##entity##Play) \
	{ \
		RB_UNUSED_PARAM; \
		const char *filename; \
		int volume = 100; \
		int pitch = 100; \
		double pos = 0.0; \
        rb_get_args(argc, argv, "z|iif", &filename, &volume, &pitch, &pos RB_ARG_END); \
		GUARD_EXC( shState->audio().entity##Play(filename, volume, pitch, pos); ) \
		return Qnil; \
	} \
	RB_METHOD(audio_##entity##Stop) \
	{ \
		RB_UNUSED_PARAM; \
		shState->audio().entity##Stop(); \
		return Qnil; \
	} \
	RB_METHOD(audio_##entity##Pos) \
	{ \
		RB_UNUSED_PARAM; \
		return rb_float_new(shState->audio().entity##Pos()); \
	}

#define DEF_PLAY_STOP(entity) \
	RB_METHOD(audio_##entity##Play) \
	{ \
		RB_UNUSED_PARAM; \
		const char *filename; \
		int volume = 100; \
		int pitch = 100; \
		rb_get_args(argc, argv, "z|ii", &filename, &volume, &pitch RB_ARG_END); \
		GUARD_EXC( shState->audio().entity##Play(filename, volume, pitch); ) \
		return Qnil; \
	} \
	RB_METHOD(audio_##entity##Stop) \
	{ \
		RB_UNUSED_PARAM; \
		shState->audio().entity##Stop(); \
		return Qnil; \
	}

#define DEF_FADE(entity) \
RB_METHOD(audio_##entity##Fade) \
{ \
	RB_UNUSED_PARAM; \
	int time; \
	rb_get_args(argc, argv, "i", &time RB_ARG_END); \
	shState->audio().entity##Fade(time); \
	return Qnil; \
}

#define DEF_POS(entity) \
	RB_METHOD(audio_##entity##Pos) \
	{ \
		RB_UNUSED_PARAM; \
		return rb_float_new(shState->audio().entity##Pos()); \
	}

// DEF_PLAY_STOP_POS( bgm )

#define MAYBE_NIL_TRACK(t) t == Qnil ? -127 : NUM2INT(t)

#define DEF_AUDIO_PROP_I(PropName) \
RB_METHOD(audio##Get##PropName) \
{ \
	RB_UNUSED_PARAM; \
	return rb_fix_new(shState->audio().get##PropName()); \
} \
RB_METHOD(audio##Set##PropName) \
{ \
	RB_UNUSED_PARAM; \
	int value; \
	rb_get_args(argc, argv, "i", &value RB_ARG_END); \
	shState->audio().set##PropName(value); \
	return rb_fix_new(value); \
}

RB_METHOD(audio_bgmPlay)
{
    RB_UNUSED_PARAM;
    const char *filename;
    int volume = 100;
    int pitch = 100;
    double pos = 0.0;
    VALUE track = Qnil;
    rb_get_args(argc, argv, "z|iifo", &filename, &volume, &pitch, &pos, &track RB_ARG_END);
    GUARD_EXC( shState->audio().bgmPlay(filename, volume, pitch, pos, MAYBE_NIL_TRACK(track)); )
    return Qnil;
}

RB_METHOD(audio_bgmStop)
{
    RB_UNUSED_PARAM;
    VALUE track = Qnil;
    rb_get_args(argc, argv, "|o", &track RB_ARG_END);
    shState->audio().bgmStop(MAYBE_NIL_TRACK(track));
    return Qnil;
}

RB_METHOD(audio_bgmPos)
{
    RB_UNUSED_PARAM;
    VALUE track = Qnil;
    rb_get_args(argc, argv, "|o", &track RB_ARG_END);
    return rb_float_new(shState->audio().bgmPos(MAYBE_NIL_TRACK(track)));
}

RB_METHOD(audio_bgmGetVolume)
{
    RB_UNUSED_PARAM;
    VALUE track = Qnil;
    rb_get_args(argc, argv, "|o", &track RB_ARG_END);
    int ret = 0;
    GUARD_EXC( ret = shState->audio().bgmGetVolume(MAYBE_NIL_TRACK(track)); )
    return rb_fix_new(ret);
}

RB_METHOD(audio_bgmSetVolume)
{
    RB_UNUSED_PARAM;
    int volume;
    VALUE track = Qnil;
    rb_get_args(argc, argv, "i|o", &volume, &track RB_ARG_END);
    GUARD_EXC( shState->audio().bgmSetVolume(volume, MAYBE_NIL_TRACK(track)); )
    return Qnil;
}

DEF_PLAY_STOP_POS( bgs )

DEF_PLAY_STOP( me )

DEF_AUDIO_PROP_I(GlobalBGMVolume)
DEF_AUDIO_PROP_I(GlobalSFXVolume)

//DEF_FADE( bgm )
RB_METHOD(audio_bgmFade)
{
    RB_UNUSED_PARAM;
    int time;
    VALUE track = Qnil;
    rb_get_args(argc, argv, "i|o", &time, &track RB_ARG_END);
    shState->audio().bgmFade(time, MAYBE_NIL_TRACK(track));
    return Qnil;
}

DEF_FADE( bgs )
DEF_FADE( me )

DEF_PLAY_STOP( se )

RB_METHOD(audioSetupMidi)
{
	RB_UNUSED_PARAM;

	shState->audio().setupMidi();

	return Qnil;
}

RB_METHOD(audioReset)
{
	RB_UNUSED_PARAM;

	shState->audio().reset();

	return Qnil;
}


#define BIND_PLAY_STOP(entity) \
	_rb_define_module_function(module, #entity "_play", audio_##entity##Play); \
	_rb_define_module_function(module, #entity "_stop", audio_##entity##Stop);

#define BIND_FADE(entity) \
	_rb_define_module_function(module, #entity "_fade", audio_##entity##Fade);

#define BIND_PLAY_STOP_FADE(entity) \
	BIND_PLAY_STOP(entity) \
	BIND_FADE(entity)

#define BIND_POS(entity) \
	_rb_define_module_function(module, #entity "_pos", audio_##entity##Pos);

#define INIT_AUDIO_PROP_BIND(PropName, prop_name_s) \
	_rb_define_module_function(module, prop_name_s, audio##Get##PropName); \
	_rb_define_module_function(module, prop_name_s "=", audio##Set##PropName); \

void
audioBindingInit()
{
	VALUE module = rb_define_module("Audio");

	BIND_PLAY_STOP_FADE( bgm );
	BIND_PLAY_STOP_FADE( bgs );
	BIND_PLAY_STOP_FADE( me  );

	_rb_define_module_function(module, "bgm_get_volume", audio_bgmGetVolume);
	_rb_define_module_function(module, "bgm_set_volume", audio_bgmSetVolume);

	INIT_AUDIO_PROP_BIND(GlobalBGMVolume, "bgm_volume");
	INIT_AUDIO_PROP_BIND(GlobalSFXVolume, "sfx_volume");

	BIND_POS( bgm );
	BIND_POS( bgs );

	_rb_define_module_function(module, "setup_midi", audioSetupMidi);

	BIND_PLAY_STOP( se )

	_rb_define_module_function(module, "__reset__", audioReset);
}
