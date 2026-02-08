// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ONESHOT_APPLE_H
#define ONESHOT_APPLE_H

#include <string>

namespace OneshotApple
{
	void desktopImageSet(std::string path, double red, double blue, double green);
	void desktopImageReset();
	void desktopImageCache();
}

#endif // ONESHOT_APPLE_H
