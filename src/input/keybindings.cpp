/* SPDX-License-Identifier: GPL-3.0-or-later
**
** This file is part of mkxp.
** Copyright (C) 2014 - 2021 Amaryllis Kulla <ancurio@mapleshrine.eu>
*/

#include "keybindings.h"

#include "config.h"
#include "util.h"

#include <stdio.h>

struct KbBindingData
{
	SDL_Scancode source;
	Input::ButtonCode target;

	void add(BDescVec &d) const
	{
		SourceDesc src;
		src.type = Key;
		src.d.scan = source;

		BindingDesc desc;
		desc.src = src;
		desc.target = target;

		d.push_back(desc);
	}
};

struct CtrlBindingData
{
    SDL_GameControllerButton source;
    Input::ButtonCode target;
    
    void add(BDescVec &d) const
    {
        SourceDesc src;
        src.type = CButton;
        src.d.cb = source;
        
        BindingDesc desc;
        desc.src = src;
        desc.target = target;
        
        d.push_back(desc);
    }
};

/* Keyboard bindings */
static const KbBindingData defaultKbBindings[] =
{
	{ SDL_SCANCODE_UP,    Input::Up    },
	{ SDL_SCANCODE_DOWN,  Input::Down  },
	{ SDL_SCANCODE_LEFT,  Input::Left  },
	{ SDL_SCANCODE_RIGHT, Input::Right },

	/*
	// mkxp keyboard bindings
	{ SDL_SCANCODE_SPACE,  Input::C     },
	{ SDL_SCANCODE_RETURN, Input::C     },
	{ SDL_SCANCODE_ESCAPE, Input::B     },
	{ SDL_SCANCODE_KP_0,   Input::B     },
	{ SDL_SCANCODE_LSHIFT, Input::A     },
	{ SDL_SCANCODE_X,      Input::B     },
	{ SDL_SCANCODE_D,      Input::Z     },
	{ SDL_SCANCODE_Q,      Input::L     },
	{ SDL_SCANCODE_W,      Input::R     },
	{ SDL_SCANCODE_A,      Input::X     },
	{ SDL_SCANCODE_S,      Input::Y     }
	*/

	// OneShot (mkxp-oneshot) keyboard bindings
	{ SDL_SCANCODE_Z,      Input::Action     },
	{ SDL_SCANCODE_SPACE,  Input::Action     },
	{ SDL_SCANCODE_X,      Input::Cancel     },
	{ SDL_SCANCODE_ESCAPE, Input::Cancel     },
	{ SDL_SCANCODE_A,      Input::Menu       },
	{ SDL_SCANCODE_RETURN, Input::Menu       },
	{ SDL_SCANCODE_S,      Input::Items      },
	{ SDL_SCANCODE_LSHIFT, Input::Run        },
	{ SDL_SCANCODE_C,      Input::Deactivate },
	{ SDL_SCANCODE_Q,      Input::L          },
	{ SDL_SCANCODE_W,      Input::R          },
};

/* RGSS1 */
/*
static const KbBindingData defaultKbBindings1[] =
{
	{ SDL_SCANCODE_Z,      Input::A     },
	{ SDL_SCANCODE_C,      Input::C     },
};
*/

/* RGSS2 and higher */
/*
static const KbBindingData defaultKbBindings2[] =
{
	{ SDL_SCANCODE_Z,      Input::C     }
};
*/

static elementsN(defaultKbBindings);
//static elementsN(defaultKbBindings1);
//static elementsN(defaultKbBindings2);

/* Game controller bindings */
static const CtrlBindingData defaultCtrlBindings[] =
{
	{ SDL_CONTROLLER_BUTTON_DPAD_UP,    Input::Up    },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN,  Input::Down  },
	{ SDL_CONTROLLER_BUTTON_DPAD_LEFT,  Input::Left  },
	{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, Input::Right },

	/*
	// mkxp bindings
	{ SDL_CONTROLLER_BUTTON_X,             Input::A },
	{ SDL_CONTROLLER_BUTTON_B,             Input::B },
	{ SDL_CONTROLLER_BUTTON_A,             Input::C },
	{ SDL_CONTROLLER_BUTTON_Y,             Input::X },
	{ SDL_CONTROLLER_BUTTON_LEFTSTICK,     Input::Y },
	{ SDL_CONTROLLER_BUTTON_RIGHTSTICK,    Input::Z },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  Input::L },
	{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, Input::R },
	*/

	// OneShot (mkxp-oneshot) bindings
	{ SDL_CONTROLLER_BUTTON_A,             Input::Action     },
	{ SDL_CONTROLLER_BUTTON_B,             Input::Cancel     },
	{ SDL_CONTROLLER_BUTTON_X,             Input::Run        },
	{ SDL_CONTROLLER_BUTTON_Y,             Input::Items      },
	{ SDL_CONTROLLER_BUTTON_START,         Input::Menu       },
	{ SDL_CONTROLLER_BUTTON_BACK,          Input::Deactivate },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  Input::L          },
	{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, Input::R          }
};

static elementsN(defaultCtrlBindings);

static void addAxisBinding(BDescVec &d, SDL_GameControllerAxis axis, AxisDir dir, Input::ButtonCode target)
{
	SourceDesc src;
	src.type = CAxis;
	src.d.ca.axis = axis;
	src.d.ca.dir = dir;

	BindingDesc desc;
	desc.src = src;
	desc.target = target;

	d.push_back(desc);
}

BDescVec genDefaultBindings(const Config &conf)
{
	BDescVec d;

	for (size_t i = 0; i < defaultKbBindingsN; ++i)
		defaultKbBindings[i].add(d);

	/*
	if (conf.rgssVersion == 1)
		for (size_t i = 0; i < defaultKbBindings1N; ++i)
			defaultKbBindings1[i].add(d);
	else
		for (size_t i = 0; i < defaultKbBindings2N; ++i)
			defaultKbBindings2[i].add(d);
	*/

	for (size_t i = 0; i < defaultCtrlBindingsN; ++i)
		defaultCtrlBindings[i].add(d);

	addAxisBinding(d, SDL_CONTROLLER_AXIS_LEFTX, Negative, Input::Left);
	addAxisBinding(d, SDL_CONTROLLER_AXIS_LEFTX, Positive, Input::Right);
	addAxisBinding(d, SDL_CONTROLLER_AXIS_LEFTY, Negative, Input::Up);
	addAxisBinding(d, SDL_CONTROLLER_AXIS_LEFTY, Positive, Input::Down);
	addAxisBinding(d, SDL_CONTROLLER_AXIS_TRIGGERLEFT, Positive, Input::Deactivate);
	addAxisBinding(d, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, Positive, Input::Run);

	return d;
}

#define FORMAT_VER 0 // 3

struct Header
{
	uint32_t formVer;
	//uint32_t rgssVer;
	uint32_t count;
};

static void buildPath(const std::string &dir, uint32_t rgssVersion, char *out, size_t outSize)
{
	//snprintf(out, outSize, "%skeybindings.mkxp%u", dir.c_str(), rgssVersion);
	snprintf(out, outSize, "%skeybindings.dat", dir.c_str());
}

static bool writeBindings(const BDescVec &d, const std::string &dir, uint32_t rgssVersion)
{
	if (dir.empty())
		return false;

	char path[1024];
	buildPath(dir, rgssVersion, path, sizeof(path));

	FILE *f = fopen(path, "wb");

	if (!f)
		return false;

	Header hd;
	hd.formVer = FORMAT_VER;
	//hd.rgssVer = rgssVersion;
	hd.count = d.size();

	if (fwrite(&hd, sizeof(hd), 1, f) < 1)
	{
		fclose(f);
		return false;
	}

	if (fwrite(&d[0], sizeof(d[0]), hd.count, f) < hd.count)
	{
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}

void storeBindings(const BDescVec &d, const Config &conf)
{
    writeBindings(d, conf.customDataPath, conf.rgssVersion);
}

#define READ(ptr, size, n, f) if (fread(ptr, size, n, f) < n) return false

static bool verifyDesc(const BindingDesc &desc)
{
	const Input::ButtonCode codes[] =
	{
	    Input::None,
	    Input::Down, Input::Left, Input::Right, Input::Up,
	    Input::Action, Input::Cancel, Input::Menu,
	    Input::Items, Input::Run, Input::Deactivate,
	    Input::L, Input::R,
	    Input::Shift, Input::Ctrl, Input::Alt,
	    Input::F5, Input::F6, Input::F7, Input::F8, Input::F9
	};

	elementsN(codes);
	size_t i;

	for (i = 0; i < codesN; ++i)
		if (desc.target == codes[i])
			break;

	if (i == codesN)
		return false;

	const SourceDesc &src = desc.src;

	switch (src.type)
	{
	case Invalid:
		return true;
	case Key:
		return src.d.scan < SDL_NUM_SCANCODES;
            
    case CButton:
        return true;

	case CAxis:
		return src.d.ca.dir == Negative || src.d.ca.dir == Positive;
	default:
		return false;
	}
}

static bool readBindings(BDescVec &out, const std::string &dir,
                         uint32_t rgssVersion)
{
	if (dir.empty())
		return false;

	char path[1024];
	buildPath(dir, rgssVersion, path, sizeof(path));

	FILE *f = fopen(path, "rb");

	if (!f)
		return false;

	Header hd;
	if (fread(&hd, sizeof(hd), 1, f) < 1)
	{
		fclose(f);
		return false;
	}

	if (hd.formVer != FORMAT_VER)
		return false;
	/*
	if (hd.rgssVer != rgssVersion)
		return false;
	*/
	/* Arbitrary max value */
	if (hd.count > 1024)
		return false;

	out.resize(hd.count);
	if (fread(&out[0], sizeof(out[0]), hd.count, f) < hd.count)
	{
		fclose(f);
		return false;
	}

	for (size_t i = 0; i < hd.count; ++i)
		if (!verifyDesc(out[i]))
			return false;

	return true;
}

BDescVec loadBindings(const Config &conf)
{
	BDescVec d;

	if (readBindings(d, conf.customDataPath, conf.rgssVersion))
		return d;

	return genDefaultBindings(conf);
}
