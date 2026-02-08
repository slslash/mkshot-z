// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef XDG_USER_DIRS_H
#define XDG_USER_DIRS_H

std::string xdgUserDirsLookup(const char *type, const char *fallback);
std::string xdgUserDirsGet(const char *type);

#endif // XDG_USER_DIRS_H
