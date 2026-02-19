#include "oneshot.h"
#include "journal.h"
#include "wallpaper.h"
#include "eventthread.h"
#include "system/system.h"
#include "util/debugwriter.h"

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
#include <dispatch/dispatch.h>
#else
#include "util/xdg-user-dirs.h"
#include "gnome-fun.h"
#endif

#include <SDL3/SDL.h>

#ifdef _WIN32
static std::string wideToUTF8(const wchar_t *wcStr)
{
	std::string ret;
	int size = WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, 0, 0, 0, 0);
	if (size > 0) {
		char *str = new char[size];
		if (WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, str, size, 0, 0) == size)
			ret = str;
		delete [] str;
	}
	return ret;
}
#endif

struct OneshotPrivate
{
	// String data
	std::string os;
	std::string lang;
	std::string userName;
	std::string savePath;
	std::string docsPath;
	std::string gamePath;
	std::string journal;

	// Exit state
	bool exiting;
	bool allowExit;

	// Dialog text
	std::string txtYes;
	std::string txtNo;

	SDL_Window *window;
	SDL_mutex *windowMutex;
	int windowPosX;
	int windowPosY;
	bool windowPosChanged;

	// Alpha texture data for portions of window obscured by screen edges
	std::vector<uint8_t> obscuredMap;
	bool obscuredCleared;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	bool gtkIsInit = false;
#endif

	OneshotPrivate()
		: exiting(false),
		  allowExit(true),
		  window(nullptr),
		  windowPosX(0),
		  windowPosY(0),
		  windowPosChanged(false),
		  obscuredCleared(false)
	{
		windowMutex = SDL_CreateMutex();
	}

	~OneshotPrivate()
	{
		SDL_DestroyMutex(windowMutex);
	}
};

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
typedef struct
{
	// Input
	int type;
	const char *body;
	const char *title;
	const char *txtYes;
	const char *txtNo;

	// Output
	int result;
} GDialogData;

static gboolean oneshot_linux_gtkdialog(gpointer user_data)
{
	GDialogData *d = reinterpret_cast<GDialogData *>(user_data);

	GtkMessageType mType;
	GtkButtonsType bType = GTK_BUTTONS_OK;

	switch (d->type)
	{
		case Oneshot::MSG_INFO:
		default:
			mType = GTK_MESSAGE_INFO;
			break;

		case Oneshot::MSG_WARN:
			mType = GTK_MESSAGE_WARNING;
			break;

		case Oneshot::MSG_ERR:
			mType = GTK_MESSAGE_ERROR;
			break;

		case Oneshot::MSG_YESNO:
			mType = GTK_MESSAGE_QUESTION;
			bType = GTK_BUTTONS_YES_NO;
			break;
	}

	GtkWidget *dialog = dynGnome.gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, mType, bType, "%s", d->body);
	dynGnome.gtk_window_set_title(DYN_GTK_WINDOW(dialog), d->title);

	// Set custom button labels
	if (bType == GTK_BUTTONS_YES_NO) {
		GtkWidget *btnYes = dynGnome.gtk_dialog_get_widget_for_response(DYN_GTK_DIALOG(dialog), GTK_RESPONSE_YES);
		GtkWidget *btnNo = dynGnome.gtk_dialog_get_widget_for_response(DYN_GTK_DIALOG(dialog), GTK_RESPONSE_NO);

		if (btnYes)
			dynGnome.gtk_button_set_label(DYN_GTK_BUTTON(btnYes), d->txtYes);
		if (btnNo)
			dynGnome.gtk_button_set_label(DYN_GTK_BUTTON(btnNo), d->txtNo);
	} /* else {
		GtkWidget *btnOk = dynGnome.gtk_dialog_get_widget_for_response(DYN_GTK_DIALOG(dialog), GTK_RESPONSE_OK);
		dynGnome.gtk_button_set_label(DYN_GTK_BUTTON(btnOk), "OK");
	} */

	// Run GTK+ dialog
	d->result = dynGnome.gtk_dialog_run(DYN_GTK_DIALOG(dialog));

	// Destroy after response
	dynGnome.gtk_widget_destroy(dialog);

	// Finish GTK+ loop
	dynGnome.gtk_main_quit();

	return FALSE;
}
#endif

Oneshot::Oneshot(RGSSThreadData &threadData) : threadData(threadData)
{
	p = new OneshotPrivate();

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	p->os = "windows";
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	p->os = "macos";
#else
	p->os = "linux";
#endif

	p->window = threadData.window;
	p->savePath = threadData.config.customDataPath.substr(0, threadData.config.customDataPath.size() - 1);

	p->lang = mkxp_sys::getLanguage();
	p->userName = mkxp_sys::getUserFullName();

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
	wchar_t wcPath[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, wcPath)))
		p->docsPath = wideToUTF8(wcPath);
	else
		p->docsPath = std::string(SDL_getenv("USERPROFILE")) + "\\Documents";

	p->gamePath = p->docsPath + "\\My Games";

	p->journal = "_______.exe";
#elif MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	std::string path = std::string(SDL_getenv("HOME")) + "/Documents";

	p->docsPath = path;
	p->gamePath = path;

	p->journal = "_______.app";
#else
	std::string path = xdgUserDirsGet("DOCUMENTS");

	p->docsPath = path;
	p->gamePath = path;

	p->journal = "_______";
#endif

	Debug() << "Game path    :" << p->gamePath;
	Debug() << "Docs path    :" << p->docsPath;

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	char const *xdg_current_desktop = SDL_getenv("XDG_CURRENT_DESKTOP");
	if (xdg_current_desktop && xdg_current_desktop[0] != '\0') {
		std::string desktop(xdg_current_desktop);
		std::transform(desktop.begin(), desktop.end(), desktop.begin(), ::tolower);

		if (desktop.find("gnome") != std::string::npos || desktop.find("unity") != std::string::npos) {
			desktopEnv = "gnome";
		} else if (desktop.find("cinnamon") != std::string::npos) {
			desktopEnv = "cinnamon";
		} else if (desktop.find("mate") != std::string::npos) {
			desktopEnv = "mate";
		} else if (desktop.find("deepin") != std::string::npos || desktop.find("dde") != std::string::npos) {
			desktopEnv = "deepin";
		} else if (desktop.find("xfce") != std::string::npos) {
			desktopEnv = "xfce";
		} else if (desktop.find("kde") != std::string::npos) {
			desktopEnv = "kde";
		} else if (desktop.find("lxde") != std::string::npos) {
			desktopEnv = "lxde";
		} else if (desktop.find("lxqt") != std::string::npos) {
			desktopEnv = "lxqt";
		} else if (desktop.find("enlightenment") != std::string::npos) {
			desktopEnv = "enlightenment";
		} else if (desktop.find("pantheon") != std::string::npos) {
			desktopEnv = "pantheon";
		} else {
			desktopEnv = "nope";
		}
	} else {
		desktopEnv = "nope";
	}

	Debug() << "Desktop env  :" << desktopEnv;

	// Init dynamic GTK+ for dialogs
	initGnomeFunctions();

	if (HAVE_GTK != NULL)
		if (dynGnome.gtk_init_check(NULL, NULL) == TRUE)
			p->gtkIsInit = true;

	// Init dynamic Gio library for wallpaper settings
	initGioFunctions();

	// Init dynamic Gdk library for displays detection
	initGdkFunctions();
#endif

	obscuredDirty = true;
	p->obscuredMap.resize(640 * 480, 255);

	journal = new Journal();
	wallpaper = new Wallpaper();
}

Oneshot::~Oneshot()
{
	delete wallpaper;
	delete journal;
	delete p;
}

void Oneshot::update()
{
	// Update obscured map data
	if (p->windowPosChanged) {
		p->windowPosChanged = false;

		// Map of unobscured pixels in this frame
		static std::vector<bool> obscuredFrame;

		obscuredFrame.resize(p->obscuredMap.size());
		std::fill(obscuredFrame.begin(), obscuredFrame.end(), true);

		SDL_Rect screenRect;

		// Get window pos
		SDL_LockMutex(p->windowMutex);
		screenRect.x = p->windowPosX;
		screenRect.y = p->windowPosY;
		SDL_UnlockMutex(p->windowMutex);

		// Get window size
		screenRect.w = 640;
		screenRect.h = 480;

		// Update obscured map and texture for window portion offscreen
		for (int i = 0, max = SDL_GetNumVideoDisplays(); i < max; ++i) {
			SDL_Rect bounds;
			SDL_GetDisplayBounds(i, &bounds);

			// Get intersection of window and the screen
			SDL_Rect intersect;

			if (!SDL_IntersectRect(&screenRect, &bounds, &intersect))
				continue;

			intersect.x -= screenRect.x;
			intersect.y -= screenRect.y;

			// If it's entirely within the bounds of the screen,
			// we don't need to check out any other monitors
			if (intersect.x == 0 && intersect.y == 0 && intersect.w == 640 && intersect.h == 480)
				return;

			for (int y = intersect.y; y < intersect.y + intersect.h; ++y) {
				int start = y * 640 + intersect.x;
				std::fill(obscuredFrame.begin() + start, obscuredFrame.begin() + (start + intersect.w), false);
			}
		}

		// Update the obscured map, and return prematurely if we don't have
		// any changes to make to the texture
		bool needsUpdate = false;
		bool cleared = true;

		for (size_t i = 0; i < obscuredFrame.size(); ++i) {
			if (obscuredFrame[i]) {
				p->obscuredMap[i] = 0;
				needsUpdate = true;
			}
			if (p->obscuredMap[i] == 255)
				cleared = false;
		}

		p->obscuredCleared = cleared;

		if (!needsUpdate)
			return;

		// Flag as dirty
		obscuredDirty = true;
	}
}

bool Oneshot::msgbox(int type, const char *body, const char *title)
{
	if (title && title[0] == '\0')
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_WINDOWS
		// Zero Width Space to remove filename of executable in message box
		// title with enabled Visual Styles (ComCtl32).
		title = "\u200b";
#else
		title = "";
#endif

#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_LINUX
	// Using GTK+ dialogs instead.
	// SDL message boxes on Linux systems are quite "ugly"...
	if (p->gtkIsInit) {
		GDialogData data;
		data.type = type;
		data.body = body;
		data.title = title;
		data.txtYes = p->txtYes.c_str();
		data.txtNo = p->txtNo.c_str();

		// Add dialog function to loop
		dynGnome.g_idle_add(oneshot_linux_gtkdialog, &data);

		// Run GTK+ loop
		dynGnome.gtk_main();

		bool result = false;
		if (data.result == GTK_RESPONSE_OK || data.result == GTK_RESPONSE_YES || data.result == GTK_RESPONSE_DELETE_EVENT)
			result = true;

		return result;
	}
#endif

	// Using SDL dialogs.

	SDL_MessageBoxButtonData buttonOk = { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "OK" };
	SDL_MessageBoxButtonData buttonYes = { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, p->txtYes.c_str() };
	SDL_MessageBoxButtonData buttonNo = { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, p->txtNo.c_str() };

	SDL_MessageBoxButtonData buttonsOk[] = { buttonOk };
	SDL_MessageBoxButtonData buttonsYesNo[] = { buttonNo, buttonYes };

	SDL_MessageBoxData data;
	data.window = NULL; // p->window
	data.title = title;
	data.message = body;
	data.colorScheme = NULL;

	switch (type)
	{
		case MSG_INFO:
		case MSG_YESNO:
		default:
			data.flags = SDL_MESSAGEBOX_INFORMATION;
			break;

		case MSG_WARN:
			data.flags = SDL_MESSAGEBOX_WARNING;
			break;

		case MSG_ERR:
			data.flags = SDL_MESSAGEBOX_ERROR;
			break;
	}

	switch (type)
	{
		case MSG_YESNO:
			data.numbuttons = 2;
			data.buttons = buttonsYesNo;
			break;

		case MSG_INFO:
		case MSG_WARN:
		case MSG_ERR:
		default:
			data.numbuttons = 1;
			data.buttons = buttonsOk;
			break;
	}

	int result;
#if MKXPZ_PLATFORM == MKXPZ_PLATFORM_MACOS
	int *btn = &result;
	// Message boxes and UI changes must be performed from the main thread
	// on macOS Mojave and above. This block ensures the message box
	// will show from the main thread.
	dispatch_sync(dispatch_get_main_queue(), ^{ SDL_ShowMessageBox(&data, btn); });
#else
	SDL_ShowMessageBox(&data, &result);
#endif

	return result ? true : false;
}

void Oneshot::setYesNo(const char *yes, const char *no)
{
	p->txtYes = yes;
	p->txtNo = no;
}

void Oneshot::setWindowPos(int x, int y)
{
	SDL_LockMutex(p->windowMutex);

	p->windowPosX = x;
	p->windowPosY = y;
	p->windowPosChanged = true;

	SDL_UnlockMutex(p->windowMutex);
}

void Oneshot::setAllowExit(bool allowExit)
{
	if (p->allowExit != allowExit) {
		p->allowExit = allowExit;

		if (allowExit)
			threadData.allowExit.set();
		else
			threadData.allowExit.clear();
	}
}

void Oneshot::setExiting(bool exiting)
{
	if (p->exiting != exiting) {
		p->exiting = exiting;

		if (exiting)
			threadData.exiting.set();
		else
			threadData.exiting.clear();
	}
}

void Oneshot::resetObscured()
{
	std::fill(p->obscuredMap.begin(), p->obscuredMap.end(), 255);
	obscuredDirty = true;
	p->obscuredCleared = false;
}

const std::string &Oneshot::os() const
{
	return p->os;
}

const std::string &Oneshot::lang() const
{
	return p->lang;
}

const std::string &Oneshot::userName() const
{
	return p->userName;
}

const std::string &Oneshot::savePath() const
{
	return p->savePath;
}

const std::string &Oneshot::docsPath() const
{
	return p->docsPath;
}

const std::string &Oneshot::gamePath() const
{
	return p->gamePath;
}

const std::string &Oneshot::journalName() const
{
	return p->journal;
}

const std::vector<uint8_t> &Oneshot::obscuredMap() const
{
	return p->obscuredMap;
}

bool Oneshot::obscuredCleared() const
{
	return p->obscuredCleared;
}

bool Oneshot::allowExit() const
{
	return p->allowExit;
}

bool Oneshot::exiting() const
{
	return p->exiting;
}
