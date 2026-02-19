/*
** filesystemImplApple.mm
** Player
**
** This file is part of mkxp-z, further modified for mkshot-z.
**
** mkxp-z is licensed under GPLv2-or-later.
** mkshot-z is licensed under GPLv3-or-later.
**
** Copyright (C) 2026 sevenleftslash <sevenleftslash@proton.me>
** Copyright (C) 2013 - 2023 mkxp-z contributors <https://github.com/mkxp-z/mkxp-z/graphs/contributors>
**
** Created by ゾロアーク on 11/21/20.
*/

#import <AppKit/AppKit.h>
// TODO: SDL_syswm.h has been removed from SDL3.
// #import <SDL_syswm.h>
#import <SDL3/SDL_filesystem.h>

#import "filesystemImpl.h"
#import "util/exception.h"

#define PATHTONS(str) [NSFileManager.defaultManager stringWithFileSystemRepresentation:str length:strlen(str)]
#define NSTOPATH(str) [NSFileManager.defaultManager fileSystemRepresentationWithPath:str]

bool filesystemImpl::fileExists(const char *path)
{
    BOOL isDir;
    return [NSFileManager.defaultManager fileExistsAtPath:PATHTONS(path) isDirectory:&isDir] && !isDir;
}

std::string filesystemImpl::contentsOfFileAsString(const char *path)
{
    NSStringEncoding enc;

    NSString *fileContents = [NSString stringWithContentsOfFile:PATHTONS(path) usedEncoding:&enc error:NULL];

    if (fileContents == nil)
        throw Exception(Exception::NoFileError, "Failed to read file at %s", path);

    return std::string(fileContents.UTF8String);
}

bool filesystemImpl::setCurrentDirectory(const char *path)
{
    return [NSFileManager.defaultManager changeCurrentDirectoryPath:PATHTONS(path)];
}

std::string filesystemImpl::getCurrentDirectory()
{
    return std::string(NSTOPATH(NSFileManager.defaultManager.currentDirectoryPath));
}

std::string filesystemImpl::normalizePath(const char *path, bool preferred, bool absolute)
{
    NSString *nsPathOrig = PATHTONS(path);
    NSString *nsPath = [NSURL fileURLWithPath:nsPathOrig].URLByStandardizingPath.path;
    NSString *pwd = [NSString stringWithFormat:@"%@/", NSFileManager.defaultManager.currentDirectoryPath];

    if ([nsPathOrig hasSuffix:@"/"])
        nsPath = [nsPath stringByAppendingString:@"/"];

    if (!absolute)
        nsPath = [nsPath stringByReplacingOccurrencesOfString:pwd withString:@""];

    nsPath = [nsPath stringByReplacingOccurrencesOfString:@"\\" withString:@"/"];

    return std::string(NSTOPATH(nsPath));
}

std::string filesystemImpl::getDefaultGameRoot()
{
    NSString *path = [NSString stringWithFormat:@"%@/../", NSBundle.mainBundle.bundlePath];
    return normalizePath(NSTOPATH(path), true, true);
}

NSString *getPathForAsset_internal(const char *baseName, const char *ext)
{
    NSString *pathBundle = [NSString stringWithFormat:@"%@/%s", NSBundle.mainBundle.resourcePath, "Assets.bundle"];
    NSBundle *assetBundle = [NSBundle bundleWithPath:pathBundle];

    if (assetBundle == nil)
        return nil;

    return [assetBundle pathForResource:@(baseName) ofType:@(ext)];
}

std::string filesystemImpl::getPathForAsset(const char *baseName, const char *ext)
{
    NSString *assetPath = getPathForAsset_internal(baseName, ext);

    if (assetPath == nil)
        throw Exception(Exception::NoFileError, "Failed to find the asset named %s.%s", baseName, ext);

    return std::string(NSTOPATH(getPathForAsset_internal(baseName, ext)));
}

std::string filesystemImpl::contentsOfAssetAsString(const char *baseName, const char *ext)
{
    NSString *path = getPathForAsset_internal(baseName, ext);
    NSString *fileContents = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:NULL];

    if (fileContents == nil)
        throw Exception(Exception::MKXPError, "Failed to read file at %s", path.UTF8String);

    return std::string(fileContents.UTF8String);
}

std::string filesystemImpl::getResourcePath()
{
    return std::string(NSTOPATH(NSBundle.mainBundle.resourcePath));
}

std::string filesystemImpl::selectPath(SDL_Window *win, const char *msg, const char *prompt)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.canChooseDirectories = true;
    panel.canChooseFiles = false;

    if (msg)
        panel.message = @(msg);

    if (prompt)
        panel.prompt = @(prompt);

    //panel.directoryURL = [NSURL fileURLWithPath:NSFileManager.defaultManager.currentDirectoryPath];

    SDL_SysWMinfo wm {};
    SDL_GetWindowWMInfo(win, &wm);

    [panel beginSheetModalForWindow:wm.info.cocoa.window completionHandler:^(NSModalResponse res) {
        [NSApp stopModalWithCode:res];
    }];

    [NSApp runModalForWindow:wm.info.cocoa.window];

    // The window needs to be brought to the front again after the OpenPanel closes
    [wm.info.cocoa.window makeKeyAndOrderFront:nil];
    if (panel.URLs.count > 0)
        return std::string(NSTOPATH(panel.URLs[0].path));

    return std::string();
}
