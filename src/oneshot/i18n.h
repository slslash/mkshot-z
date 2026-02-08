// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ONESHOT_I18N
#define ONESHOT_I18N

#include <string>

namespace OneshotImpl
{
	namespace i18n
	{
		void loadLocale(const char *locale);
		void unloadLocale();

		void loadLanguageMetadata();
		void unloadLanguageMetadata();

		std::string findtext(const char *message);

		std::string getFontName();
		int getFontSize();

		std::string stringUnescape(const std::string &input);
	}
}

#endif // ONESHOT_I18N
