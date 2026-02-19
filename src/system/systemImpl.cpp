/*
** systemImpl.cpp
**
** This file is part of mkxp-z, further modified for mkshot-z.
**
** mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2013 - 2023 mkxp-z contributors <https://github.com/mkxp-z/mkxp-z/graphs/contributors>
*/

#include "system.h"

#include <string>
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <windows.h>
#include <security.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_loadso.h>

#ifdef _WIN32
static std::string wideToUTF8(const wchar_t *wcStr)
{
	std::string ret;
	int size = WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, 0, 0, 0, 0);
	if (size > 0) {
		char *str = new char[size];
		if (WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, str, size, 0, 0) == size)
			ret = str;
		delete [] str;
	}
	return ret;
}
#endif

std::string systemImpl::getLanguage()
{
	std::string ret;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	wchar_t wcBuf[18] = {'\0'};

	wchar_t wcLang[9];
	wchar_t wcCountry[9];

	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, wcLang, sizeof(wcLang) / sizeof(wchar_t));
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, wcCountry, sizeof(wcCountry) / sizeof(wchar_t));

	swprintf(wcBuf, sizeof(wcBuf), L"%ls_%ls", wcLang, wcCountry);

	ret = wideToUTF8(wcBuf);
#else
	const char *locFallback = "en_US";
	const char *loc;

	loc = SDL_getenv("LANG");

	if (!loc || (loc && loc[0] == '\0'))
		loc = SDL_getenv("LC_MESSAGES");

	if (!loc || (loc && loc[0] == '\0'))
		loc = SDL_getenv("LC_ALL");

	if (loc && loc[0] != '\0') {
		std::string tmpLoc(loc);
		std::string::size_type tmpPos;

		// Chomp next XPG part after dot (codeset and modifier)
		tmpPos = tmpLoc.find('.');
		if (tmpPos != std::string::npos)
			tmpLoc.resize(tmpPos);

		if (tmpLoc == "C" || tmpLoc == "POSIX")
			ret = locFallback;
		else
			ret = tmpLoc;
	} else {
		ret = locFallback;
	}
#endif

	return ret;
}

std::string systemImpl::getUserName()
{
	std::string ret;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	DWORD size = 0;
	GetUserNameW(0, &size);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		wchar_t *wcBuf = new wchar_t[size];
		GetUserNameW(wcBuf, &size);
		ret = wideToUTF8(wcBuf);
		delete [] wcBuf;
	}
#else
	uid_t userId;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	userId = geteuid();
#else
	userId = getuid();
#endif

	struct passwd *pw = getpwuid(userId);

	if (pw && (pw->pw_name && pw->pw_name[0] != '\0'))
		ret = pw->pw_name;
#endif

	if (ret.empty()) {
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
		const char *user = SDL_getenv("USERNAME");
#else
		const char *user = SDL_getenv("USER");
#endif

		if (user && user[0] != '\0')
			ret = user;
		else
			ret = "noname";
	}

	return ret;
}

std::string systemImpl::getUserFullName()
{
	std::string ret;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	ULONG size = 0;
	GetUserNameExW(NameDisplay, 0, &size);
	if (GetLastError() == ERROR_MORE_DATA) {
		wchar_t *wcBuf = new wchar_t[size];
		GetUserNameExW(NameDisplay, wcBuf, &size);
		ret = wideToUTF8(wcBuf);
		delete [] wcBuf;
	}
#else
	uid_t userId;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	userId = geteuid();
#else
	userId = getuid();
#endif

	struct passwd *pw = getpwuid(userId);

	if (pw && (pw->pw_gecos && pw->pw_gecos[0] != '\0' && pw->pw_gecos[0] != ',')) {
		std::string tmpGecos(pw->pw_gecos);
		std::string::size_type tmpPos;

		// Chomp next GECOS part after comma,
		// we only need to get only the full name
		tmpPos = tmpGecos.find(',');
		if (tmpPos != std::string::npos)
			tmpGecos.resize(tmpPos);

		ret = tmpGecos;
	}
#endif

	// Fallback to login name if we couldn't get the full name
	if (ret.empty())
		ret = getUserName();

	return ret;
}

int systemImpl::getScalingFactor()
{
	// HiDPI scaling not supported outside of macOS for now
	return 1;
}

bool systemImpl::isWine()
{
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	void *ntdll = SDL_LoadObject("ntdll.dll");
	return SDL_LoadFunction(ntdll, "wine_get_host_version") != 0;
#else
	// Always false on non-Windows builds
	return false;
#endif
}

bool systemImpl::isRosetta()
{
	// Always false on non-Mac environment, see systemImplApple.mm
	return false;
}

systemImpl::WineHostType systemImpl::getRealHostType()
{
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	void *ntdll = SDL_LoadObject("ntdll.dll");

	void (*wine_get_host_version)(const char **, const char **) =
	(void (*)(const char **, const char **))SDL_LoadFunction(ntdll, "wine_get_host_version");

	if (wine_get_host_version == 0)
		return WineHostType::Windows;

	const char *kernel = 0;
	wine_get_host_version(&kernel, 0);

	if (!strcmp(kernel, "Darwin"))
		return WineHostType::Mac;

	return WineHostType::Linux;
#else
	// Always Linux host type on builds for Linux
	return WineHostType::Linux;
#endif
}
