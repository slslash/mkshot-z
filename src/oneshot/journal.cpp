/*
** journal.cpp
**
** This file is part of ModShot-mkxp-z, further modified for mkshot-z.
**
** ModShot-mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2024 hat_kid <https://github.com/thehatkid>
*/

#include "journal.h"
#include "eventthread.h"
#include "sharedstate.h"
#include "system/system.h"
#include "filesystem/filesystem.h"
#include "debugwriter.h"

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#include <SDL3/SDL.h>

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
// TODO: SDL_syswm.h has been removed from SDL3.
// #include <SDL_syswm.h>

static std::wstring utf8ToWide(const char *str)
{
	std::wstring ret;
	if (str && str[0] != '\0') {
		int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, 0, 0);
		if (size > 0) {
			wchar_t *wStr = new wchar_t[size];
			if (MultiByteToWideChar(CP_UTF8, 0, str, -1, wStr, size) == size)
				ret = wStr;
			delete [] wStr;
		}
	}
	return ret;
}
#endif

#define JOURNAL_BUFFER_SIZE 256

#define NIKO_X (320 - 16)
#define NIKO_Y ((13 * 16) * 2)

struct JournalData
{
	volatile int msgLen;
	volatile char msgBuf[JOURNAL_BUFFER_SIZE];

#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
	std::string pipePath;
	int pipeFD;
#endif

	SDL_Thread *thread;
	SDL_mutex *mutex;
};

struct JournalPrivate
{
	volatile bool active;

	volatile char langBuf[JOURNAL_BUFFER_SIZE];

	JournalData journal;

#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
	JournalData niko;
#endif

	JournalPrivate()
		: active(false)
	{
		langBuf[0] = '_';

		journal.thread = nullptr;
		journal.mutex = SDL_CreateMutex();

		memset((char *)journal.msgBuf, 0, JOURNAL_BUFFER_SIZE);

#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
		niko.thread = nullptr;
		niko.mutex = SDL_CreateMutex();

		memset((char *)niko.msgBuf, 0, JOURNAL_BUFFER_SIZE);

		std::string home;

		struct passwd *pw = getpwuid(getuid());

		if (pw && (pw->pw_name && pw->pw_dir[0] != '\0'))
			home = pw->pw_dir;

		if (home.empty()) {
			const char *homeEnv = SDL_getenv("HOME");
			if (homeEnv && homeEnv[0] != '\0')
				home = homeEnv;
			else
				home = "/tmp";
		}

		journal.pipePath = home + "/.oneshot-pipe";
		journal.pipeFD = -1;

		niko.pipePath = home + "/.oneshot-niko-pipe";
		niko.pipeFD = -1;
#endif
	}

	~JournalPrivate()
	{
		SDL_DestroyMutex(journal.mutex);

#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
		SDL_DestroyMutex(niko.mutex);
#endif
	}
};

int journal_server(void *data)
{
	JournalPrivate *p = static_cast<JournalPrivate *>(data);

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	HANDLE pipe = CreateNamedPipeW(
		L"\\\\.\\pipe\\oneshot-journal-to-game",
		PIPE_ACCESS_OUTBOUND,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		JOURNAL_BUFFER_SIZE, JOURNAL_BUFFER_SIZE,
		0,
		NULL
	);

	for (;;) {
		ConnectNamedPipe(pipe, NULL);

		SDL_LockMutex(p->journal.mutex);

		p->active = true;

		DWORD written;
		if (!WriteFile(pipe, (const void *)p->journal.msgBuf, JOURNAL_BUFFER_SIZE, &written, NULL))
			Debug() << "Failure writing to Journal's pipe!";

		SDL_UnlockMutex(p->journal.mutex);

		FlushFileBuffers(pipe);
		DisconnectNamedPipe(pipe);
	}

	CloseHandle(pipe);
#else
	if (access(p->journal.pipePath.c_str(), F_OK) != -1) {
		p->journal.pipeFD = open(
			p->journal.pipePath.c_str(),
			O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
		);

		if (p->journal.pipeFD == -1) {
			Debug() << "Failure to access Journal's pipe!";
		} else {
			SDL_LockMutex(p->journal.mutex);

			p->active = true;

			if (p->journal.msgLen > 0) {
				if (write(p->journal.pipeFD, (char *)p->journal.msgBuf, p->journal.msgLen) == -1)
					Debug() << "Failure writing to Journal's pipe!";
			}

			SDL_UnlockMutex(p->journal.mutex);
		}
	}
#endif

	return 0;
}

#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
int journal_niko_server(void *data)
{
	JournalPrivate *p = static_cast<JournalPrivate *>(data);

	if (access(p->niko.pipePath.c_str(), F_OK) != -1) {
		p->niko.pipeFD = open(
			p->niko.pipePath.c_str(),
			O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
		);

		SDL_LockMutex(p->niko.mutex);

		if (p->niko.msgLen > 0) {
			if (write(p->niko.pipeFD, (char *)p->niko.msgBuf, p->niko.msgLen) == -1)
				Debug() << "Failure writing to Niko's pipe!";
		}

		SDL_UnlockMutex(p->niko.mutex);
	}

	return 0;
}
#endif

Journal::Journal()
{
	p = new JournalPrivate();

#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
	mkfifo(p->journal.pipePath.c_str(), 0666);
	mkfifo(p->niko.pipePath.c_str(), 0666);
#endif
}

Journal::~Journal()
{
#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
	unlink(p->journal.pipePath.c_str());
	remove(p->journal.pipePath.c_str());
	unlink(p->niko.pipePath.c_str());
	remove(p->niko.pipePath.c_str());
#endif

	delete p;
}

bool Journal::isActive() const
{
	return p->active;
}

void Journal::set(const char *name)
{
	SDL_LockMutex(p->journal.mutex);

	p->journal.msgLen = strlen(name);
	strcpy((char *)p->journal.msgBuf, name);

	// In the case where Journal is being sent empty string, do not append
	// the language suffix, because empty string is the signifier
	// to terminate the Journal program.
	if (p->journal.msgLen > 0) {
		strcpy((char *)p->journal.msgBuf + p->journal.msgLen, (char *)p->langBuf);
		p->journal.msgLen += strlen((char *)p->langBuf);
	}

	SDL_UnlockMutex(p->journal.mutex);

	// Write message to the Journal pipe
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	HANDLE pipe = CreateFileW(
		L"\\\\.\\pipe\\oneshot-game-to-journal",
		GENERIC_WRITE, 0,
		NULL,
		OPEN_EXISTING, 0,
		NULL
	);

	if (pipe != INVALID_HANDLE_VALUE) {
		p->active = true;

		DWORD written;
		if (!WriteFile(pipe, (const void *)p->journal.msgBuf, JOURNAL_BUFFER_SIZE, &written, NULL))
			Debug() << "Failure writing to Journal's pipe!";

		FlushFileBuffers(pipe);
		CloseHandle(pipe);
	}

	// Create thread for new Journal connections
	if (!p->journal.thread)
		p->journal.thread = SDL_CreateThread(journal_server, "journal", p);
#else
	// Clean up connection thread
	if (p->journal.thread && p->journal.pipeFD != -1) {
		SDL_WaitThread(p->journal.thread, NULL);
		p->journal.thread = nullptr;
	}

	if (p->journal.pipeFD != -1) {
		// We have a connection, so write it
		if (write(p->journal.pipeFD, (char *)p->journal.msgBuf, p->journal.msgLen) <= 0) {
			// In the case of an error, close
			close(p->journal.pipeFD);
			p->journal.pipeFD = -1;
		}
	} else {
		// We don't have a pipe open, so create connection thread
		p->journal.thread = SDL_CreateThread(journal_server, "journal", p);
	}
#endif
}

void Journal::setLang(const char *lang)
{
	strcpy((char *)p->langBuf + 1, lang);
}

void Journal::nikoPrepare()
{
#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
	std::string name;
	std::string cwd = mkxp_fs::getCurrentDirectory();

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	name = "_______.app/Contents/MacOS/_______";
#else
	name = "_______";
#endif

	std::string path = cwd + "/" + name;

	pid_t pid = fork();

	if (pid < 0) {
		Debug() << "Couldn't fork for Journal!";
	} else if (pid == 0) {
		// Child processing
		if (execl(path.c_str(), path.c_str(), "niko", (char *)NULL) < 0) {
			Debug() << "Failed to launch Journal!";
			exit(1);
		} else {
			exit(0);
		}
	}
#endif

	return;
}

void Journal::nikoStart()
{
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	// Calculate where to stick the window
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(shState->rtData().window, &wmInfo);

	POINT pos;
	pos.x = NIKO_X;
	pos.y = NIKO_Y;
	ClientToScreen(wmInfo.info.win.window, &pos);

	// Prepare process information
	std::string name = "_______.exe";

	std::string cwd = mkxp_fs::getCurrentDirectory();
	std::string path = cwd + "\\" + name;

	std::wstring wPath = utf8ToWide(path.c_str());
	std::wstring wCwd = utf8ToWide(cwd.c_str());

	wchar_t wArgs[512] = {'\0'};
	swprintf(wArgs, sizeof(wArgs), L"\"%ls\" %ld %ld", wPath.c_str(), pos.x, pos.y);

	// Start process
	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	BOOL result = CreateProcessW(wPath.c_str(), wArgs, NULL, NULL, FALSE, 0, NULL, wCwd.c_str(), &si, &pi);

	if (!result)
		Debug() << "Failed to start Journal! Error:" << GetLastError();
#else
	// Calculate where to stick the window
	int x;
	int y;

	SDL_GetWindowPosition(shState->rtData().window, &x, &y);

	x += NIKO_X;
	y += NIKO_Y;

	// Prepare message
	char message[32] = {'\0'};
	snprintf(message, sizeof(message), "%d,%d\n", x, y);

	SDL_LockMutex(p->niko.mutex);

	p->niko.msgLen = strlen(message);
	strcpy((char *)p->niko.msgBuf, message);

	SDL_UnlockMutex(p->niko.mutex);

	// Attempt to send
	if (p->niko.thread && p->niko.pipeFD != -1) {
		SDL_WaitThread(p->niko.thread, NULL);
		p->niko.thread = nullptr;
	}

	if (p->niko.pipeFD != -1) {
		if (write(p->niko.pipeFD, (char *)p->niko.msgBuf, p->niko.msgLen) <= 0) {
			close(p->niko.pipeFD);
			p->niko.pipeFD = -1;
		}
	} else {
		p->niko.thread = SDL_CreateThread(journal_niko_server, "journal-niko", p);
	}
#endif
}
