/*
** oneshot-binding.cpp
**
** This file is part of ModShot-mkxp-z, further modified for mkshot-z.
**
** ModShot-mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2024 hat_kid <https://github.com/thehatkid>
*/

#include "binding-util.h"
#include "binding-types.h"
#include "sharedstate.h"
#include "eventthread.h"
#include "system/system.h"
#include "oneshot/oneshot.h"

#include <SDL3/SDL.h>
#include "util/CRC.h"

RB_METHOD(oneshotSetYesNo)
{
	RB_UNUSED_PARAM;

	const char *yes;
	const char *no;

	rb_get_args(argc, argv, "zz", &yes, &no RB_ARG_END);

	shState->oneshot().setYesNo(yes, no);

	return Qnil;
}

RB_METHOD(oneshotMsgBox)
{
	RB_UNUSED_PARAM;

	int type;
	const char *body;
	const char *title = "";

	rb_get_args(argc, argv, "iz|z", &type, &body, &title RB_ARG_END);

	return rb_bool_new(shState->oneshot().msgbox(type, body, title));
}

RB_METHOD(oneshotResetObscured)
{
	RB_UNUSED_PARAM;

	shState->oneshot().resetObscured();

	return Qnil;
}

RB_METHOD(oneshotObscuredCleared)
{
	RB_UNUSED_PARAM;

	return shState->oneshot().obscuredCleared() ? Qtrue : Qfalse;
}

RB_METHOD(oneshotAllowExit)
{
	RB_UNUSED_PARAM;

	bool allowExit;

	rb_get_args(argc, argv, "b", &allowExit RB_ARG_END);

	shState->oneshot().setAllowExit(allowExit);

	return Qnil;
}

RB_METHOD(oneshotExiting)
{
	RB_UNUSED_PARAM;

	bool exiting;

	rb_get_args(argc, argv, "b", &exiting RB_ARG_END);

	shState->oneshot().setExiting(exiting);

	return Qnil;
}

RB_METHOD(oneshotShake)
{
	RB_UNUSED_PARAM;

	int absX;
	int absY;

	SDL_GetWindowPosition(shState->rtData().window, &absX, &absY);

	int state;
	srand(time(NULL));

	for (int i = 0; i < 60; ++i) {
		int max = 60 - i;
		int x = rand() % (max * 2) - max;
		int y = rand() % (max * 2) - max;

		SDL_SetWindowPosition(shState->rtData().window, absX + x, absY + y);

		rb_eval_string_protect("sleep 0.02", &state);
	}

	return Qnil;
}

RB_METHOD(oneshotCRC32)
{
	RB_UNUSED_PARAM;

	VALUE string;

	rb_get_args(argc, argv, "S", &string RB_ARG_END);

	std::string str = std::string(RSTRING_PTR(string), RSTRING_LEN(string));
	std::uint32_t crc = CRC::Calculate(str.data(), str.length(), CRC::CRC_32());

	return UINT2NUM(crc);
}

void oneshotBindingInit()
{
	VALUE module = rb_define_module("Oneshot");
	VALUE moduleMsg = rb_define_module_under(module, "Msg");

	rb_const_set(module, rb_intern("OS"), rb_str_new2(shState->oneshot().os().c_str()));

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	rb_const_set(module, rb_intern("DE"), rb_str_new2(shState->oneshot().desktopEnv.c_str()));
#endif

	rb_const_set(module, rb_intern("USER_NAME"), rb_str_new2(shState->oneshot().userName().c_str()));
	rb_const_set(module, rb_intern("SAVE_PATH"), rb_str_new2(shState->oneshot().savePath().c_str()));
	rb_const_set(module, rb_intern("DOCS_PATH"), rb_str_new2(shState->oneshot().docsPath().c_str()));
	rb_const_set(module, rb_intern("GAME_PATH"), rb_str_new2(shState->oneshot().gamePath().c_str()));
	rb_const_set(module, rb_intern("JOURNAL"), rb_str_new2(shState->oneshot().journalName().c_str()));
	rb_const_set(module, rb_intern("LANG"), rb_str_new2(shState->oneshot().lang().c_str()));

	rb_const_set(moduleMsg, rb_intern("INFO"), INT2FIX(Oneshot::MSG_INFO));
	rb_const_set(moduleMsg, rb_intern("YESNO"), INT2FIX(Oneshot::MSG_YESNO));
	rb_const_set(moduleMsg, rb_intern("WARN"), INT2FIX(Oneshot::MSG_WARN));
	rb_const_set(moduleMsg, rb_intern("ERR"), INT2FIX(Oneshot::MSG_ERR));

	_rb_define_module_function(module, "set_yes_no", oneshotSetYesNo);
	_rb_define_module_function(module, "msgbox", oneshotMsgBox);
	_rb_define_module_function(module, "reset_obscured", oneshotResetObscured);
	_rb_define_module_function(module, "obscured_cleared?", oneshotObscuredCleared);
	_rb_define_module_function(module, "allow_exit", oneshotAllowExit);
	_rb_define_module_function(module, "exiting", oneshotExiting);
	_rb_define_module_function(module, "shake", oneshotShake);
	_rb_define_module_function(module, "crc32", oneshotCRC32);
}
