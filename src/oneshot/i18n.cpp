// SPDX-License-Identifier: GPL-3.0-or-later

#include "i18n.h"
#include "util/sdl-util.h"
#include "util/boost-hash.h"

namespace OneshotImpl
{
	namespace i18n
	{
		std::string currentLocale;
		BoostHash<std::string, std::string> poTranslations;
		BoostHash<std::string, std::string> langFonts;
		BoostHash<std::string, int> langSizes;

		void loadLocale(const char *locale)
		{
			unloadLocale();

			currentLocale = std::string(locale);

			SDLRWStream poFile(std::string("Languages/internal/" + currentLocale + ".po").c_str(), "r");

			if (poFile) {
				std::string msgId;
				std::string msgStr;

				for (std::string line; std::getline(poFile.stream(), line);)
				{
					std::string::size_type pos = 0;
					std::string::size_type match = std::string::npos;

					// Skip whitespace and left trim the line
					while (line[pos] == ' ' || line[pos] == '\t')
						pos++;

					if (pos > 0) {
						line = line.substr(pos);
						pos = 0;
					}

					// Skip empty line
					if (line.length() <= 0)
						continue;

					// Skip comment line
					if (line[pos] == '#')
						continue;

					// Parse Original Text
					match = line.find("msgid");
					if (match != std::string::npos) {
						pos = match + 5;

						match = line.find("\"");
						if (match != std::string::npos) {
							pos = match + 1;

							std::string content = line.substr(pos);

							// Find ending quote of content
							for (std::string::size_type i = 0; i < content.size(); i++)
							{
								if (content[i] == '\\')
									// Internal escaping
									i++;
								else if (content[i] == '"')
									// End of line
									content.resize(i);
							}

							msgId = stringUnescape(content);

							continue;
						}
					}

					// Parse Translated Text
					match = line.find("msgstr");
					if (match != std::string::npos) {
						pos = match + 6;

						match = line.find("\"");
						if (match != std::string::npos) {
							pos = match + 1;

							std::string content = line.substr(pos);

							// Find ending quote of content
							for (std::string::size_type i = 0; i < content.size(); i++)
							{
								if (content[i] == '\\')
									// Internal escaping
									i++;
								else if (content[i] == '"')
									// End of line
									content.resize(i);
							}

							msgStr = stringUnescape(content);

							if (!msgId.empty() && !msgStr.empty()) {
								poTranslations.insert(msgId, msgStr);
								msgId.clear();
								msgStr.clear();
							}

							continue;
						}
					}
				}
			}
		}

		void unloadLocale()
		{
			poTranslations.clear();
			currentLocale.clear();
		}

		void loadLanguageMetadata()
		{
			SDLRWStream iniFontsFile("Languages/internal/language_fonts.ini", "r");
			SDLRWStream iniSizesFile("Languages/internal/language_sizes.ini", "r");

			if (iniFontsFile) {
				std::string lang;
				std::string font;

				for (std::string line; std::getline(iniFontsFile.stream(), line);)
				{
					std::string::size_type pos = 0;
					std::string::size_type match = std::string::npos;

					// Skip whitespace and left trim the line
					while (line[pos] == ' ' || line[pos] == '\t')
						pos++;

					if (pos > 0) {
						line = line.substr(pos);
						pos = 0;
					}

					// Skip empty line
					if (line.length() <= 0)
						continue;

					// Skip comment line
					if (line[pos] == '#' || line[pos] == ';')
						continue;

					// Parse the line
					match = pos;

					while (line[pos] && line[pos] != '=')
						pos++;

					if (line.length() <= pos)
						continue;

					lang = line.substr(match, pos);

					pos++;

					while (line[pos] && (line[pos] == ' ' || line[pos] == '\t'))
						pos++;

					font = line.substr(pos);

					langFonts.insert(lang, font);
				}
			}

			if (iniSizesFile) {
				std::string lang;
				int size;

				for (std::string line; std::getline(iniSizesFile.stream(), line);)
				{
					std::string::size_type pos = 0;
					std::string::size_type match = std::string::npos;

					// Skip whitespace and left trim the line
					while (line[pos] == ' ' || line[pos] == '\t')
						pos++;

					if (pos > 0) {
						line = line.substr(pos);
						pos = 0;
					}

					// Skip empty line
					if (line.length() <= 0)
						continue;

					// Skip comment line
					if (line[pos] == '#' || line[pos] == ';')
						continue;

					// Parse the line
					match = pos;

					while (line[pos] && line[pos] != '=')
						pos++;

					if (line.length() <= pos)
						continue;

					lang = line.substr(match, pos);

					pos++;

					while (line[pos] && (line[pos] == ' ' || line[pos] == '\t'))
						pos++;

					size = std::stoi(line.substr(pos));

					langSizes.insert(lang, size);
				}
			}
		}

		void unloadLanguageMetadata()
		{
			langFonts.clear();
			langSizes.clear();
		}

		std::string findtext(const char *message)
		{
			return poTranslations.value(message, message);
		}

		std::string getFontName()
		{
			return langFonts.value(currentLocale, "Terminus (TTF)");
		}

		int getFontSize()
		{
			return langSizes.value(currentLocale, 12);
		}

		std::string stringUnescape(const std::string &input)
		{
			std::string result;

			result.reserve(input.size());

			for (size_t i = 0; i < input.length(); i++)
			{
				if (input[i] == '\\' && i + 1 < input.length()) {
					switch (input[i + 1])
					{
						case '\\':
							result += '\\';
							i++;
							break;

						case '"':
							result += '"';
							i++;
							break;

						case 'n':
							result += '\n';
							i++;
							break;

						case 't':
							result += '\t';
							i++;
							break;

						default:
							result += input[i];
							break;
					}
				} else {
					result += input[i];
				}
			}

			return result;
		}
	}
}
