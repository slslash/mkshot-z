// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ONESHOT_H
#define ONESHOT_H

#include "etc-internal.h"

#include <string>

struct RGSSThreadData;

struct OneshotPrivate;

class Journal;
class Wallpaper;

class Oneshot
{
private:
	OneshotPrivate *p;
	RGSSThreadData &threadData;

public:
	Oneshot(RGSSThreadData &threadData);
	~Oneshot();

	Journal *journal;
	Wallpaper *wallpaper;

	// Message box types
	enum
	{
		MSG_INFO,
		MSG_YESNO,
		MSG_WARN,
		MSG_ERR
	};

	// Functions
	void update();
	bool msgbox(int type, const char *body, const char *title);

	// Mutators
	void setYesNo(const char *yes, const char *no);
	void setWindowPos(int x, int y);
	void setAllowExit(bool allowExit);
	void setExiting(bool exiting);
	void resetObscured();

	// Accessors
	const std::string &os() const;
	const std::string &lang() const;
	const std::string &userName() const;
	const std::string &savePath() const;
	const std::string &docsPath() const;
	const std::string &gamePath() const;
	const std::string &journalName() const;
	const std::vector<uint8_t> &obscuredMap() const;
	bool obscuredCleared() const;
	bool allowExit() const;
	bool exiting() const;

	// Dirty flag for Obscured texture
	bool obscuredDirty;

	// Linux: Desktop Environment string
	std::string desktopEnv;
};

#endif // ONESHOT_H
