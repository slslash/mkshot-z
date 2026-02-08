// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STEAM_H
#define STEAM_H

#include <string>

struct SteamPrivate;

class Steam
{
private:
	friend struct SharedStatePrivate;

	SteamPrivate *p;

	Steam();
	~Steam();

public:
	void unlock(const char *name);
	void lock(const char *name);
	bool isUnlocked(const char *name);

	const std::string &userName() const;
	const std::string &lang() const;
};

#endif // STEAM_H
