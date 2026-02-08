// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaper.h"
#include "system/system.h"
#include "filesystem/filesystem.h"
#include "debugwriter.h"

#include <string>

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
#include <vector>
#include <map>
#include <fstream>
#include <iomanip>

#include "gnome-fun.h"
#include "xfconf-fun.h"
#include "util/xdg-user-dirs.h"
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
#include "oneshot-apple.h"
#endif

#ifdef MKXPZ_EXP_FS
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include "filesystem/ghc/filesystem.hpp"
namespace fs = ghc::filesystem;
#endif

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
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

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
struct XfceRGBA
{
	double red;
	double green;
	double blue;
	double alpha;
};
#endif

struct WallpaperPrivate
{
	bool isCached = false;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	bool setStyle = false;
	bool setTile = false;

	wchar_t szFile[MAX_PATH + 1] = {0};
	wchar_t szStyle[9] = {0};
	wchar_t szTile[9] = {0};
	DWORD bgColor = 0;

	DWORD szStyleSize = sizeof(szStyle) - 1;
	DWORD szTileSize = sizeof(szTile) - 1;
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	std::string desktop;
	std::string fallbackPath;

	// GNOME properties
	GSettings *gSettings = NULL;
	std::string gPictureURI;
	std::string gPictureDarkURI;
	std::string gPictureOptions;
	std::string gPrimaryColor;
	std::string gColorShadingType;

	// Xfce properties
	bool xfInit = false;
	XfconfChannel *xfChannel = NULL;
	std::vector<std::string> xfMonitors;
	std::vector<std::string> xfPropsImage;
	std::vector<std::string> xfPropsImageStyle;
	std::vector<std::string> xfPropsColorStyle;
	std::vector<std::string> xfPropsColorRGBA1;
	std::vector<std::string> xfImageURIs;
	std::vector<int> xfImageStyles;
	std::vector<int> xfColorStyles;
	std::vector<bool> xfColorExists;
	std::vector<XfceRGBA> xfColorValues = {};

	// KDE properties
	std::string kdeConfigPath;
	std::map<std::string, std::string> kdePlugins;
	std::map<std::string, std::string> kdeImages;
	std::map<std::string, std::string> kdeColors;
	std::map<std::string, std::string> kdeFillModes;
	std::map<std::string, bool> kdeBlurs;

	// LXDE/LXQt properties
	std::string lxWallpaperPath;
#endif

	WallpaperPrivate() { }
	~WallpaperPrivate() { }
};

Wallpaper::Wallpaper()
{
	p = new WallpaperPrivate();

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	p->fallbackPath = xdgUserDirsGet("DESKTOP") + "/ONESHOT_hint.png";

	const char *xdg_current_desktop = SDL_getenv("XDG_CURRENT_DESKTOP");
	if (xdg_current_desktop && xdg_current_desktop[0] != '\0') {
		std::string desktop(xdg_current_desktop);
		std::transform(desktop.begin(), desktop.end(), desktop.begin(), ::tolower);

		if (desktop.find("gnome") != std::string::npos || desktop.find("unity") != std::string::npos) {
			p->desktop = "gnome";
		} else if (desktop.find("cinnamon") != std::string::npos) {
			p->desktop = "cinnamon";
		} else if (desktop.find("mate") != std::string::npos) {
			p->desktop = "mate";
		} else if (desktop.find("deepin") != std::string::npos || desktop.find("dde") != std::string::npos) {
			p->desktop = "deepin";
		} else if (desktop.find("xfce") != std::string::npos) {
			p->desktop = "xfce";
		} else if (desktop.find("kde") != std::string::npos) {
			p->desktop = "kde";
		// TODO
		/* } else if (desktop.find("lxde") != std::string::npos) {
			p->desktop = "lxde";
		} else if (desktop.find("lxqt") != std::string::npos) {
			p->desktop = "lxqt";
		} else if (desktop.find("enlightenment") != std::string::npos) {
			p->desktop = "enlightenment";
		} else if (desktop.find("pantheon") != std::string::npos) {
			p->desktop = "pantheon"; */
		} else {
			p->desktop = "nope";
		}
	} else {
		p->desktop = "nope";
	}

	if (p->desktop == "gnome" || p->desktop == "cinnamon" || p->desktop == "mate" || p->desktop == "deepin") {
		if (HAVE_GIO != NULL) {
			// Get GSettings instance
			if (p->desktop == "gnome")
				p->gSettings = dynGio.g_settings_new("org.gnome.desktop.background");
			else if (p->desktop == "cinnamon")
				p->gSettings = dynGio.g_settings_new("org.cinnamon.desktop.background");
			else if (p->desktop == "mate")
				p->gSettings = dynGio.g_settings_new("org.mate.background");
			else if (p->desktop == "deepin")
				// FIXME: Fix wallpaper changing on Deepin DE (com.deepin.dde.appearance)
				p->gSettings = dynGio.g_settings_new("com.deepin.wrap.gnome.desktop.background");
		} else {
			p->desktop = p->desktop + "_error";
		}
	} else if (p->desktop == "xfce") {
		initXfconfFunctions();

		if (HAVE_XFCONF != NULL) {
			// Obtain monitor names
			const char *displayVar = SDL_getenv("DISPLAY");
			GdkDisplay *display = dynGdk.gdk_display_open(displayVar);
			int numberOfDisplays = dynGdk.gdk_display_get_n_monitors(GDK_DISPLAY(display));

			for (int i = 0; i < numberOfDisplays; i++) {
				GdkMonitor *monitor = dynGdk.gdk_display_get_monitor(GDK_DISPLAY(display), i);
				std::string name = dynGdk.gdk_monitor_get_model(monitor);
				name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
				p->xfMonitors.push_back(name);
			}

			// Initialize dynamically Xfconf
			GError *xfconfError = NULL;
			if (dynXfconf.init(&xfconfError)) {
				p->xfInit = true;
				p->xfChannel = dynXfconf.channel_get("xfce4-desktop");

				// Prepare all property paths for each monitors
				std::string propPrefix = "/backdrop/screen0/monitor";
				std::string propSuffix = "/workspace0/";
				std::string propImage = "last-image";
				std::string propImageStyle = "image-style";
				std::string propColorStyle = "color-style";
				std::string propColorRGBA1 = "rgba1";

				for (uint i = 0; i < p->xfMonitors.size(); i++) {
					std::string propPath = propPrefix + p->xfMonitors.at(i) + propSuffix;
					p->xfPropsImage.push_back(propPath + propImage);
					p->xfPropsImageStyle.push_back(propPath + propImageStyle);
					p->xfPropsColorStyle.push_back(propPath + propColorStyle);
					p->xfPropsColorRGBA1.push_back(propPath + propColorRGBA1);
				}
			} else {
				Debug() << "Could not initialize Xfconf:" << xfconfError->message;
				p->desktop = "xfce_error";
				dynGnome.g_error_free(xfconfError);
			}
		} else {
			p->desktop = "xfce_error";
		}
	} else if (p->desktop == "kde") {
		p->kdeConfigPath = xdgUserDirsGet("HOME") + "/.config/plasma-org.kde.plasma.desktop-appletsrc";

		std::ifstream config;

		config.open(p->kdeConfigPath, std::ios::in);

		if (!config.is_open()) {
			Debug() << "Could not open KDE desktop configuration";
			p->desktop = "kde_error";
		}

		config.close();
	}
#endif
}

Wallpaper::~Wallpaper()
{
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	if (p->xfInit && HAVE_XFCONF != NULL)
		dynXfconf.shutdown();
#endif

	delete p;
}

void Wallpaper::set(const char *name, int color)
{
	fs::path pathFS(name);

	if (!pathFS.is_absolute())
		pathFS = mkxp_fs::getCurrentDirectory() + "/Wallpaper/" + std::string(name);

	if (!pathFS.has_extension())
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
		pathFS += ".bmp";
#else
		pathFS += ".png";
#endif

#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
	// Replace "w32" with "unix" in filename
	std::string filename = pathFS.filename();
	if (filename.rfind("w32") != std::string::npos) {
		filename.replace(filename.rfind("w32"), 3, "unix");
		pathFS.replace_filename(filename);
	}
#endif

	Debug() << "Setting wallpaper to" << pathFS.c_str();

	std::string path = mkxp_fs::normalizePath(pathFS.c_str(), true, true);

	cache();

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	std::wstring wPath = utf8ToWide(path.c_str());

	wchar_t wpStyle[2] = {L'0', L'\0'};
	wchar_t wpTile[2] = {L'0', L'\0'};
	int colorId = COLOR_BACKGROUND;

	HKEY hKey = NULL;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
		Debug() << "Could not open Registry key for write";
	} else {
		if (RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, (const BYTE *)wpStyle, sizeof(wpStyle)) != ERROR_SUCCESS)
			Debug() << "Failure to set wallpaper style";

		if (RegSetValueExW(hKey, L"TileWallpaper", 0, REG_SZ, (const BYTE *)wpTile, sizeof(wpTile)) != ERROR_SUCCESS)
			Debug() << "Failure to set wallpaper tile state";
	}

	RegCloseKey(hKey);
	hKey = NULL;

	if (!SetSysColors(1, &colorId, (const COLORREF *)&color))
		Debug() << "Failure to set background color";

	if (!SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)wPath.c_str(), SPIF_UPDATEINIFILE))
		Debug() << "Failure to set wallpaper image";
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	if (p->desktop == "gnome" || p->desktop == "cinnamon" || p->desktop == "mate" || p->desktop == "deepin") {
		// Set wallpaper via GSettings (dconf)

		if (p->gSettings != NULL) {
			// Convert decimal to HEX string color
			std::stringstream hexColor;
			if (color > 16777215)
				hexColor << "#ffffff";
			else if (color < 0)
				hexColor << "#000000";
			else
				hexColor << "#" << std::setfill('0') << std::setw(6) << std::hex << color;

			// Set new background picture URI
			if (p->desktop == "mate") {
				dynGio.g_settings_set_string(p->gSettings, "picture-filename", path.c_str());
			} else {
				dynGio.g_settings_set_string(p->gSettings, "picture-uri", ("file://" + path).c_str());

				// Set also background picture URI for Dark theme on GNOME
				if (p->desktop == "gnome") {
					// Check for key existence
					GSettingsSchemaSource *source = dynGio.g_settings_schema_source_get_default();
					GSettingsSchema *schema = dynGio.g_settings_schema_source_lookup(source, "org.gnome.desktop.background", TRUE);

					if (dynGio.g_settings_schema_has_key(schema, "picture-uri-dark"))
						dynGio.g_settings_set_string(p->gSettings, "picture-uri-dark", ("file://" + path).c_str());
				}
			}

			// Set new background options
			dynGio.g_settings_set_string(p->gSettings, "picture-options", "scaled");
			dynGio.g_settings_set_string(p->gSettings, "primary-color", hexColor.str().c_str());
			dynGio.g_settings_set_string(p->gSettings, "color-shading-type", "solid");
		}
	} else if (p->desktop == "xfce") {
		// Set wallpaper via Xfconf channel

		if (p->xfChannel != NULL) {
			// Convert decimal color to RGBA double values
			int r = (color >> 16) & 0xFF;
			int g = (color >> 8) & 0xFF;
			int b = color & 0xFF;
			double dr = (r * 256 + r) / 65535.0;
			double dg = (g * 256 + g) / 65535.0;
			double db = (b * 256 + b) / 65535.0;
			double da = 1.0;

			// Set new backdrop on each monitor
			for (uint i = 0; i < p->xfMonitors.size(); i++) {
				dynXfconf.channel_set_string(p->xfChannel, p->xfPropsImage.at(i).c_str(), path.c_str());
				dynXfconf.channel_set_int(p->xfChannel, p->xfPropsImageStyle.at(i).c_str(), 4); // Scaled
				dynXfconf.channel_set_int(p->xfChannel, p->xfPropsColorStyle.at(i).c_str(), 0); // Solid
				dynXfconf.channel_set_array(
					p->xfChannel,
					p->xfPropsColorRGBA1.at(i).c_str(),
					G_TYPE_DOUBLE, &dr,
					G_TYPE_DOUBLE, &dg,
					G_TYPE_DOUBLE, &db,
					G_TYPE_DOUBLE, &da,
					G_TYPE_INVALID
				);
			}
		}
	} else if (p->desktop == "kde") {
		// Set wallpaper via Qt D-Bus command

		// Convert decimal color to RGB values
		int r = color & 0xFF;
		int g = (color >> 8) & 0xFF;
		int b = (color >> 16) & 0xFF;
		std::string rStr = std::to_string(r);
		std::string gStr = std::to_string(g);
		std::string bStr = std::to_string(b);

		std::string command;
		command = "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript '"
			"var a = desktops(); "
			"for (i = 0; i < a.length; i++) { "
				"d = a[i]; "
				"d.wallpaperPlugin = \"org.kde.image\"; "
				"d.currentConfigGroup = Array(\"Wallpaper\", \"org.kde.image\", \"General\"); "
				"d.writeConfig(\"Color\", Array(\"" + rStr + "\", \"" + gStr + "\", \"" + bStr + "\")); "
				"d.writeConfig(\"Blur\", false); "
				"d.writeConfig(\"FillMode\", \"1\"); "
				"d.writeConfig(\"Image\", \"file://" + path + "\"); "
			"}'";

		Debug() << "Set wallpaper command:" << command;

		if (system(command.c_str()) != 0)
			Debug() << "Failure to execute qdbus command";
	} else {
		// Fallback, writing picture file to Desktop user directory
		std::ifstream srcHint(path);
		std::ofstream dstHint(p->fallbackPath);
		dstHint << srcHint.rdbuf();
		dstHint.close();
		srcHint.close();
	}
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	double r = ((color >> 16) & 0xFF) / 255.0;
	double g = ((color >> 8) & 0xFF) / 255.0;
	double b = (color & 0xFF) / 255.0;

	OneshotApple::desktopImageSet(path, r, g, b);
#endif
}

void Wallpaper::reset()
{
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	if (p->desktop.empty() || p->desktop == "nope" || p->desktop.rfind("error") != std::string::npos) {
		// Delete fallback image file
		if (access(p->fallbackPath.c_str(), F_OK) == 0)
			if (remove(p->fallbackPath.c_str()) != 0)
				Debug() << "Failed to delete" << p->fallbackPath;
	}
#endif

	if (!p->isCached)
		return;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	int colorId = COLOR_BACKGROUND;

	HKEY hKey = NULL;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
		Debug() << "Could not open Registry key for write";
	} else {
		if (p->setStyle)
			RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, (const BYTE *)p->szStyle, p->szStyleSize);

		if (p->setTile)
			RegSetValueExW(hKey, L"TileWallpaper", 0, REG_SZ, (const BYTE *)p->szTile, p->szTileSize);
	}

	RegCloseKey(hKey);
	hKey = NULL;

	if (!SetSysColors(1, &colorId, (const COLORREF *)&p->bgColor))
		Debug() << "Failure to set old background color";

	if (!SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)p->szFile, SPIF_UPDATEINIFILE))
		Debug() << "Failure to set old wallpaper image";
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	if (p->desktop == "gnome" || p->desktop == "cinnamon" || p->desktop == "mate" || p->desktop == "deepin") {
		// Reset wallpaper via GSettings (dconf)

		if (p->gSettings) {
			// Set old background picture URI
			if (p->desktop == "mate") {
				dynGio.g_settings_set_string(p->gSettings, "picture-filename", p->gPictureURI.c_str());
			} else {
				dynGio.g_settings_set_string(p->gSettings, "picture-uri", p->gPictureURI.c_str());

				// Also set old background picture URI for Dark theme on GNOME
				if (p->desktop == "gnome" && !p->gPictureDarkURI.empty()) {
					// Check for key existence
					GSettingsSchemaSource *source = dynGio.g_settings_schema_source_get_default();
					GSettingsSchema *schema = dynGio.g_settings_schema_source_lookup(source, "org.gnome.desktop.background", TRUE);

					if (dynGio.g_settings_schema_has_key(schema, "picture-uri-dark"))
						dynGio.g_settings_set_string(p->gSettings, "picture-uri-dark", p->gPictureDarkURI.c_str());
				}
			}

			// Set old background options
			dynGio.g_settings_set_string(p->gSettings, "picture-options", p->gPictureOptions.c_str());
			dynGio.g_settings_set_string(p->gSettings, "primary-color", p->gPrimaryColor.c_str());
			dynGio.g_settings_set_string(p->gSettings, "color-shading-type", p->gColorShadingType.c_str());
		}
	} else if (p->desktop == "xfce") {
		// Reset wallpaper via Xfconf channel

		if (p->xfChannel != NULL) {
			for (uint i = 0; i < p->xfMonitors.size(); i++) {
				if (p->xfImageURIs.at(i) != "")
					dynXfconf.channel_set_string(p->xfChannel, p->xfPropsImage.at(i).c_str(), p->xfImageURIs.at(i).c_str());
				else
					dynXfconf.channel_reset_property(p->xfChannel, p->xfPropsImage.at(i).c_str(), false);

				if (p->xfImageStyles.at(i) != -1)
					dynXfconf.channel_set_int(p->xfChannel, p->xfPropsImageStyle.at(i).c_str(), p->xfImageStyles.at(i));
				else
					dynXfconf.channel_reset_property(p->xfChannel, p->xfPropsImageStyle.at(i).c_str(), false);

				if (p->xfColorStyles.at(i) != -1)
					dynXfconf.channel_set_int(p->xfChannel, p->xfPropsColorStyle.at(i).c_str(), p->xfColorStyles.at(i));
				else
					dynXfconf.channel_reset_property(p->xfChannel, p->xfPropsColorStyle.at(i).c_str(), false);

				if (p->xfColorExists.at(i))
					dynXfconf.channel_set_array(
						p->xfChannel,
						p->xfPropsColorRGBA1.at(i).c_str(),
						G_TYPE_DOUBLE, &p->xfColorValues.at(i).red,
						G_TYPE_DOUBLE, &p->xfColorValues.at(i).green,
						G_TYPE_DOUBLE, &p->xfColorValues.at(i).blue,
						G_TYPE_DOUBLE, &p->xfColorValues.at(i).alpha,
						G_TYPE_INVALID
					);
				else
					dynXfconf.channel_reset_property(p->xfChannel, p->xfPropsColorRGBA1.at(i).c_str(), false);
			}
		}
	} else if (p->desktop == "kde") {
		// Reset wallpaper via Qt D-Bus command

		std::stringstream command;
		command << "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript '" <<
			"var a = desktops(); " <<
			"var data = {";

		// Plugin, image, color, mode, blur
		for (auto const &x : p->kdePlugins) {
			command << "\"" << x.first << "\": {plugin: \"" << x.second << "\"";

			if (p->kdeImages.find(x.first) != p->kdeImages.end()) {
				std::string imagePath = p->kdeImages[x.first];

				// Escape backslashes in image path
				std::string::size_type strpos = 0;
				while ((strpos = imagePath.find('\\', strpos)) != std::string::npos) {
					imagePath.replace(strpos, 1, "\\\\");
					strpos += 2;
				}
				strpos = 0;
				while ((strpos = imagePath.find('"', strpos)) != std::string::npos) {
					imagePath.replace(strpos, 1, "\\\"");
					strpos += 2;
				}
				strpos = 0;
				while ((strpos = imagePath.find('\'', strpos)) != std::string::npos) {
					imagePath.replace(strpos, 1, "\\x27");
					strpos += 2;
				}
				strpos = 0;

				command << ", image: \"" << imagePath << "\"";
			}

			if (p->kdeColors.find(x.first) != p->kdeColors.end())
				command << ", color: \"" << p->kdeColors[x.first] << "\"";

			if (p->kdeFillModes.find(x.first) != p->kdeFillModes.end())
				command << ", mode: \"" << p->kdeFillModes[x.first] << "\"";

			if (p->kdeBlurs.find(x.first) != p->kdeBlurs.end() && p->kdeBlurs[x.first])
				command << ", blur: true";

			command << "}, ";
		}

		command << "\"no\": {}}; " <<
			"for (var i = 0, l = a.length; i < l; ++i) { " <<
				"var d = a[i]; " <<
				"var dat = data[d.id]; " <<
				"d.wallpaperPlugin = dat.plugin; " <<
				"d.currentConfigGroup = Array(\"Wallpaper\", dat.plugin, \"General\"); " <<
				"if (dat.color) { d.writeConfig(\"Color\", dat.color.split(\",\")); } " <<
				"if (dat.mode) { d.writeConfig(\"FillMode\", dat.mode); } " <<
				"if (dat.blur) { d.writeConfig(\"Blur\", dat.blur); } " <<
				"if (dat.image) { d.writeConfig(\"Image\", dat.image); } " <<
				"d.reloadConfig(); " <<
			"}'";

		Debug() << "Reset wallpaper command:" << command.str();

		if (system(command.str().c_str()) != 0)
			Debug() << "Failure to execute qdbus command";
	}
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	OneshotApple::desktopImageReset();
#endif
}

void Wallpaper::cache()
{
	if (p->isCached)
		return;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	HKEY hKey = NULL;

	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		Debug() << "Could not open Registry key for read";
	} else {
		p->setStyle = (RegQueryValueExW(hKey, L"WallpaperStyle", NULL, NULL, (LPBYTE)p->szStyle, &p->szStyleSize) == ERROR_SUCCESS);
		p->setTile = (RegQueryValueExW(hKey, L"TileWallpaper", NULL, NULL, (LPBYTE)p->szTile, &p->szTileSize) == ERROR_SUCCESS);
	}

	RegCloseKey(hKey);
	hKey = NULL;

	p->bgColor = GetSysColor(COLOR_BACKGROUND);

	if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, p->szFile, 0))
		p->isCached = true;
	else
		Debug() << "Failure to get current wallpaper image";
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	if (p->desktop == "gnome" || p->desktop == "cinnamon" || p->desktop == "mate" || p->desktop == "deepin") {
		// Get wallpaper via GSettings (dconf)

		if (p->gSettings != NULL) {
			// Get background picture URI
			if (p->desktop == "mate")
				p->gPictureURI = dynGio.g_settings_get_string(p->gSettings, "picture-filename");
			else
				p->gPictureURI = dynGio.g_settings_get_string(p->gSettings, "picture-uri");

			// Get also background picture URI for Dark theme on GNOME
			if (p->desktop == "gnome") {
				// Check for key existence
				GSettingsSchemaSource *source = dynGio.g_settings_schema_source_get_default();
				GSettingsSchema *schema = dynGio.g_settings_schema_source_lookup(source, "org.gnome.desktop.background", TRUE);

				if (dynGio.g_settings_schema_has_key(schema, "picture-uri-dark"))
					p->gPictureDarkURI = dynGio.g_settings_get_string(p->gSettings, "picture-uri-dark");
			}

			// Get background options
			p->gPictureOptions = dynGio.g_settings_get_string(p->gSettings, "picture-options");
			p->gPrimaryColor = dynGio.g_settings_get_string(p->gSettings, "primary-color");
			p->gColorShadingType = dynGio.g_settings_get_string(p->gSettings, "color-shading-type");

			p->isCached = true;
		}
	} else if (p->desktop == "xfce") {
		// Get wallpaper via Xfconf channel

		if (p->xfChannel != NULL) {
			for (uint i = 0; i < p->xfMonitors.size(); i++) {
				// Get backdrop image and style
				p->xfImageURIs.push_back(dynXfconf.channel_get_string(p->xfChannel, p->xfPropsImage.back().c_str(), ""));
				p->xfImageStyles.push_back(dynXfconf.channel_get_int(p->xfChannel, p->xfPropsImageStyle.back().c_str(), -1));

				// Get backdrop color and style
				p->xfColorStyles.push_back(dynXfconf.channel_get_int(p->xfChannel, p->xfPropsColorStyle.back().c_str(), -1));
				p->xfColorValues.push_back({});
				p->xfColorExists.push_back(dynXfconf.channel_get_array(
					p->xfChannel,
					p->xfPropsColorRGBA1.back().c_str(),
					G_TYPE_DOUBLE, &p->xfColorValues.back().red,
					G_TYPE_DOUBLE, &p->xfColorValues.back().green,
					G_TYPE_DOUBLE, &p->xfColorValues.back().blue,
					G_TYPE_DOUBLE, &p->xfColorValues.back().alpha,
					G_TYPE_INVALID
				));
			}

			p->isCached = true;
		}
	} else if (p->desktop == "kde") {
		// Get (parse) wallpaper from KDE desktop config file

		std::ifstream config;

		config.open(p->kdeConfigPath, std::ios::in);

		if (!config.is_open()) {
			Debug() << "Could not open KDE desktop configuration";
		} else {
			std::string line;
			std::string containment;
			std::vector<std::string> sections;
			bool readPlugin = false;
			bool readOther = false;

			while (std::getline(config, line))
			{
				std::string::size_type index = std::string::npos;
				std::string::size_type lastIndex = std::string::npos;

				if (line.size() <= 0) {
					readPlugin = false;
					readOther = false;
					continue;
				}

				if (readPlugin) {
					index = line.find('=');

					if (line.substr(0, index) == "wallpaperplugin")
						p->kdePlugins[containment] = line.substr(index + 1);
				} else if (readOther) {
					index = line.find('=');

					std::string key = line.substr(0, index);
					std::string value = line.substr(index + 1);

					if (key == "Image")
						p->kdeImages[containment] = value;
					else if (key == "Color")
						p->kdeColors[containment] = value;
					else if (key == "FillMode")
						p->kdeFillModes[containment] = value;
					else if (key == "Blur")
						p->kdeBlurs[containment] = (value == "true");
				} else if (line.at(0) == '[') {
					sections.clear();

					while (true)
					{
						index = line.find(lastIndex == std::string::npos ? '[' : ']', index == std::string::npos ? 0 : index);

						if (index == std::string::npos)
							break;

						if (lastIndex == std::string::npos) {
							lastIndex = index;
						} else {
							sections.push_back(line.substr(lastIndex + 1, index - lastIndex - 1));
							lastIndex = std::string::npos;
						}
					}

					if (sections.size() == 2 && sections[0] == "Containments") {
						readPlugin = true;
						containment = sections[1];
					} else if (
						sections.size() == 5 &&
						sections[0] == "Containments" &&
						sections[2] == "Wallpaper" &&
						(sections[3] == "org.kde.image" || sections[3] == "org.kde.color") &&
						sections[4] == "General"
					) {
						readOther = true;
						containment = sections[1];
					}
				}
			}

			config.close();

			p->isCached = true;
		}
	}
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	OneshotApple::desktopImageCache();

	p->isCached = true;
#endif
}
