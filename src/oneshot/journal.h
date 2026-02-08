// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ONESHOT_JOURNAL_H
#define ONESHOT_JOURNAL_H

struct JournalPrivate;

class Journal
{
private:
	JournalPrivate *p;

public:
	Journal();
	~Journal();

	bool isActive() const;
	void set(const char *name);
	void setLang(const char *lang);

	void nikoPrepare();
	void nikoStart();
};

#endif // ONESHOT_JOURNAL_H
