#include "steam.h"
#include "steamshim/steamshim_child.h"

#include <map>
#include <SDL3/SDL.h>

/* All achievement API names to check each for unlocked state */
static const char *const achievementNames[] = {
	"CHAOTIC_EVIL",
	"SHOCK",
	"EXTREME_BARTERING",
	"RAM_WHISPERER",
	"PANCAKES",
	"WE_RIDE_AT_DAWN",
	"SECRET",
	"BOOKWORM",
	"REBIRTH",
	"ONESHOT",
	"RETURN"
};

#define NUM_ACHIEVEMENTS (sizeof(achievementNames) / sizeof(achievementNames[0]))

/* Steam to ISO-like languages map */
static inline std::map<std::string, std::string> mapLangToCode()
{
	std::map<std::string, std::string> langs;

	langs["arabic"] = "ar";
	langs["brazilian"] = "pt_BR";
	langs["bulgarian"] = "bg";
	langs["czech"] = "cs";
	langs["danish"] = "da";
	langs["dutch"] = "nl";
	langs["english"] = "en";
	langs["finnish"] = "fi";
	langs["french"] = "fr";
	langs["german"] = "de";
	langs["greek"] = "el";
	langs["hungarian"] = "hu";
	langs["indonesian"] = "id";
	langs["italian"] = "it";
	langs["japanese"] = "ja";
	langs["koreana"] = "ko";
	langs["latam"] = "es_419";
	langs["norwegian"] = "no";
	langs["polish"] = "pl";
	langs["portuguese"] = "pt";
	langs["romanian"] = "ro";
	langs["russian"] = "ru";
	langs["schinese"] = "zh_CN";
	langs["spanish"] = "es";
	langs["swedish"] = "sv";
	langs["tchinese"] = "zh_TW";
	langs["thai"] = "th";
	langs["turkish"] = "tr";
	langs["ukrainian"] = "uk";
	langs["vietnamese"] = "vn";

	return langs;
}

static const std::map<std::string, std::string> langToCode = mapLangToCode();

static std::string steamToIsoLang(const char *lang)
{
	std::map<std::string, std::string>::const_iterator it = langToCode.find(lang);

	if (it != langToCode.end())
		return it->second;

	return "en";
}

struct SteamPrivate
{
	std::string userName;
	std::string lang;
	std::map<std::string, bool> achievements;

	SteamPrivate()
	{
		STEAMSHIM_getPersonaName();
		STEAMSHIM_getCurrentGameLanguage();

		for (size_t i = 0; i < NUM_ACHIEVEMENTS; ++i)
			STEAMSHIM_getAchievement(achievementNames[i]);

		while (!initialized())
		{
			SDL_Delay(100);
			update();
		}
	}

	bool initialized()
	{
		return !userName.empty() && !lang.empty() && achievements.size() == NUM_ACHIEVEMENTS;
	}

	void update()
	{
		const STEAMSHIM_Event *e;
		while ((e = STEAMSHIM_pump()) != 0)
		{
			switch (e->type)
			{
				case SHIMEVENT_GETPERSONANAME:
					userName = e->name;
					break;

				case SHIMEVENT_GETCURRENTGAMELANGUAGE:
					lang = steamToIsoLang(e->name);
					break;

				case SHIMEVENT_GETACHIEVEMENT:
					updateAchievement(e->name, e->ivalue);
					break;

				default:
					break;
			}
		}
	}

	void setAchievement(const char *name, bool set)
	{
		achievements[name] = set;

		STEAMSHIM_setAchievement(name, set);
		STEAMSHIM_storeStats();
	}

	void updateAchievement(const char *name, bool isSet)
	{
		achievements[name] = isSet;
	}

	bool isAchievementSet(const char *name)
	{
		return achievements[name];
	}
};

Steam::Steam()
{
	p = new SteamPrivate();
}

Steam::~Steam()
{
	delete p;
}

void Steam::unlock(const char *name)
{
	p->setAchievement(name, true);
}

void Steam::lock(const char *name)
{
	p->setAchievement(name, false);
}

bool Steam::isUnlocked(const char *name)
{
	return p->isAchievementSet(name);
}

const std::string &Steam::userName() const
{
	return p->userName;
}

const std::string &Steam::lang() const
{
	return p->lang;
}
