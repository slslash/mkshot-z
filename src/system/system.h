// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_H
#define SYSTEM_H

#define MKXPZ_PLATFORM_WINDOWS 0
#define MKXPZ_PLATFORM_MACOS 1
#define MKXPZ_PLATFORM_LINUX 2

#ifdef _WIN32
#define MKXPZ_PLATFORM MKXPZ_PLATFORM_WINDOWS
#elif defined __APPLE__
#define MKXPZ_PLATFORM MKXPZ_PLATFORM_MACOS
#elif defined __linux__
#define MKXPZ_PLATFORM MKXPZ_PLATFORM_LINUX
#else
#error "Can't identify platform."
#endif

#include <string>

namespace systemImpl
{
	enum WineHostType
	{
		Windows,
		Linux,
		Mac
	};

	std::string getLanguage();
	std::string getUserName();
	std::string getUserFullName();

	int getScalingFactor();

	bool isWine();
	bool isRosetta();
	WineHostType getRealHostType();
}

#ifdef MKXPZ_BUILD_XCODE
std::string getPlistValue(const char *key);
void openSettingsWindow();
bool isMetalSupported();
#endif

namespace mkxp_sys = systemImpl;

#endif // SYSTEM_H
