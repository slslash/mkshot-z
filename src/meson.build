# Get project dependencies
dep_zlib = dependency('zlib', static: opt_static)
dep_physfs = dependency('physfs', version: '>=2.1', static: opt_static)
dep_iconv = cc.find_library('iconv', dirs: [mkxpz_deps_prefix / 'lib'], static: true)
dep_uchardet = dependency('uchardet', static: opt_static)
dep_ogg = dependency('ogg', static: opt_static)
dep_vorbis = dependency('vorbis', static: opt_static)
dep_vorbisfile = dependency('vorbisfile', static: opt_static)
dep_theora = dependency('theora', static: opt_static)
dep_png = dependency('libpng', static: opt_static)
dep_jpeg = dependency('libjpeg', static: opt_static)
dep_pixman = dependency('pixman-1', static: opt_static)
dep_openal = dependency('openal', method: 'pkg-config', static: opt_static)
dep_sdl3 = dependency('SDL3', static: opt_static)
dep_sdl3_image = dependency('SDL3_image', static: opt_static)
dep_sdl3_ttf = dependency('SDL3_ttf', static: opt_static)
dep_sdl3_sound = dependency('SDL3_sound', static: opt_static)

mkxpz_dependencies += [
    dep_zlib, dep_physfs, dep_iconv, dep_uchardet,
    dep_ogg, dep_vorbis, dep_vorbisfile, dep_theora,
    dep_png, dep_jpeg, dep_pixman, dep_openal,
    dep_sdl3, dep_sdl3_image, dep_sdl3_ttf, dep_sdl3_sound
]

# Include FluidSynth when option is true
if opt_fluidsynth
    dep_fluidsynth = dependency('fluidsynth', static: opt_static)
    mkxpz_dependencies += dep_fluidsynth
    mkxpz_cflags += '-DSHARED_FLUID'

    # FluidSynth also wants DirectSound library on Windows
    if host_system == 'windows'
        mkxpz_dependencies += cc.find_library('dsound', required: true)
    endif
endif

# Include OpenSSL when option is true
if get_option('https') == true
    dep_openssl = dependency('openssl', required: false, static: opt_static)
    if dep_openssl.found()
        mkxpz_dependencies += dep_openssl
        mkxpz_cflags += '-DMKXPZ_SSL'
    else
        warning('Could not locate OpenSSL library; HTTPS support remains disabled')
    endif
endif

# Win32 API: Required for GetUserNameEx
if host_system == 'windows'
    mkxpz_dependencies += cc.find_library('Secur32', required: true)
endif

mkxpz_includes += include_directories(
    '.',
    'audio',
    'crypto',
    'display',
    'display/gl',
    'display/libnsgif',
    'display/libnsgif/utils',
    'etc',
    'filesystem',
    'filesystem/ghc',
    'input',
    'net',
    'oneshot',
    'system',
    'util',
    'util/sigslot',
    'util/sigslot/adapter'
)

mkxpz_sources += files(
    'main.cpp',
    'config.cpp',
    'eventthread.cpp',
    'sharedstate.cpp',
    'settingsmenu.cpp',

    'audio/alstream.cpp',
    'audio/audio.cpp',
    'audio/audiostream.cpp',
    'audio/fluid-fun.cpp',
    'audio/midisource.cpp',
    'audio/sdlsoundsource.cpp',
    'audio/soundemitter.cpp',
    'audio/vorbissource.cpp',

    'crypto/rgssad.cpp',

    'display/autotiles.cpp',
    'display/autotilesvx.cpp',
    'display/bitmap.cpp',
    'display/font.cpp',
    'display/graphics.cpp',
    'display/plane.cpp',
    'display/sprite.cpp',
    'display/tilemap.cpp',
    'display/tilemapvx.cpp',
    'display/viewport.cpp',
    'display/window.cpp',
    'display/windowvx.cpp',
    'display/gl/gl-debug.cpp',
    'display/gl/gl-fun.cpp',
    'display/gl/gl-meta.cpp',
    'display/gl/glstate.cpp',
    'display/gl/scene.cpp',
    'display/gl/shader.cpp',
    'display/gl/texpool.cpp',
    'display/gl/tileatlas.cpp',
    'display/gl/tileatlasvx.cpp',
    'display/gl/tilequad.cpp',
    'display/gl/vertex.cpp',
    'display/libnsgif/libnsgif.c',
    'display/libnsgif/lzw.c',

    'etc/etc.cpp',
    'etc/table.cpp',

    'filesystem/filesystem.cpp',
    'filesystem/filesystemImpl.cpp',

    'input/input.cpp',
    'input/keybindings.cpp',

    'net/LUrlParser.cpp',
    'net/net.cpp',

    'oneshot/i18n.cpp',
    'oneshot/oneshot.cpp',
    'oneshot/journal.cpp',
    'oneshot/wallpaper.cpp',

    'system/systemImpl.cpp',

    'theoraplay/theoraplay.c',

    'util/iniconfig.cpp',
    'util/win-consoleutils.cpp'
)

if mkxpz_steam
    mkxpz_includes += include_directories('steam')
    mkxpz_sources += files('steam/steam.cpp')
endif

if host_system == 'linux'
    mkxpz_sources += files(
        'util/xdg-user-dirs.cpp',
        'oneshot/gnome-fun.cpp',
        'oneshot/xfconf-fun.cpp'
    )
endif
