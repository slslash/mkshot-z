// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIN_CONSOLEUTIL_H
#define WIN_CONSOLEUTIL_H

#include <iostream>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

bool setupWindowsConsole();

void reopenWindowsStreams();

int getStdFD(const DWORD &nStdHandle);

#endif
