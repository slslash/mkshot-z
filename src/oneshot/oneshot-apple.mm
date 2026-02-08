// SPDX-License-Identifier: GPL-3.0-or-later

#include "oneshot-apple.h"

#include <string>

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

BOOL isCached = NO;
NSURL *cachedImageURL;
NSDictionary<NSWorkspaceDesktopImageOptionKey, id> *cachedImageOptions;

NSScreen *screen = [NSScreen mainScreen];
NSWorkspace *sharedworkspace = [NSWorkspace sharedWorkspace];

void OneshotApple::desktopImageSet(std::string path, double red, double green, double blue)
{
    NSURL *url = [NSURL fileURLWithPath:@(path.c_str())];
    NSDictionary<NSWorkspaceDesktopImageOptionKey, id> *options = @{
        NSWorkspaceDesktopImageScalingKey : @3,
        NSWorkspaceDesktopImageAllowClippingKey : @0,
        NSWorkspaceDesktopImageFillColorKey : [NSColor colorWithSRGBRed:red green:green blue:blue alpha:1.0]
    };

    NSError *error = nil;
    BOOL success = [sharedworkspace setDesktopImageURL:[url absoluteURL] forScreen:screen options:options error:&error];
    if (!success)
        NSLog(@"Failure to set new desktop image: %@", error);
}

void OneshotApple::desktopImageReset()
{
    if (!isCached)
        return;

    NSError *error = nil;
    BOOL success = [sharedworkspace setDesktopImageURL:cachedImageURL forScreen:screen options:cachedImageOptions error:&error];
    if (!success)
        NSLog(@"Failure to set old desktop image: %@", error);
}

void OneshotApple::desktopImageCache()
{
    if (isCached)
        return;

    cachedImageURL = [sharedworkspace desktopImageURLForScreen:screen];
    cachedImageOptions = [sharedworkspace desktopImageOptionsForScreen:screen];
    isCached = YES;
}
