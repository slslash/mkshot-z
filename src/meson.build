# Get project dependencies
dep_zlib = dependency('zlib', static: is_static)
dep_physfs = dependency('physfs', version: '>=2.1', static: is_static)
dep_iconv = cc.find_library('iconv', dirs: [mkxp_deps_prefix / 'lib'], static: true)
dep_uchardet = dependency('uchardet', static: is_static)

dep_ogg = dependency('ogg', static: is_static)
dep_vorbis = dependency('vorbis', static: is_static)
dep_vorbisfile = dependency('vorbisfile', static: is_static)
dep_theora = dependency('theora', static: is_static)

dep_png = dependency('libpng', static: is_static)
dep_jpeg = dependency('libjpeg', static: is_static)
dep_pixman = dependency('pixman-1', static: is_static)

dep_openal = dependency('openal', method: 'pkg-config', static: is_static)

dep_sdl2 = dependency('SDL3', static: is_static)
dep_sdl2_image = dependency('SDL3_image', static: is_static)
dep_sdl2_ttf = dependency('SDL3_ttf', static: is_static)
dep_sdl2_sound = dependency('SDL3_sound', static: is_static)

mkxp_dependencies += [
    dep_zlib, dep_physfs, dep_iconv, dep_uchardet,
    dep_ogg, dep_vorbis, dep_vorbisfile, dep_theora,
    dep_png, dep_jpeg, dep_pixman, dep_openal,
    dep_sdl2, dep_sdl2_image, dep_sdl2_ttf, dep_sdl2_sound
]

# Define OpenAL ALCdevice structure name
alcdevice_struct = 'ALCdevice_struct'
if dep_openal.version().version_compare('>=1.20.0')
    alcdevice_struct = 'ALCdevice'
endif
mkxp_cflags += '-DMKXPZ_ALCDEVICE=@0@'.format(alcdevice_struct)

# Include FluidSynth when option is true
if get_option('fluidsynth') == true
    dep_fluidsynth = dependency('fluidsynth', static: is_static)
    mkxp_dependencies += dep_fluidsynth
    mkxp_cflags += '-DSHARED_FLUID'

    # FluidSynth also wants DirectSound library on Windows
    if host_system == 'windows'
        mkxp_dependencies += cc.find_library('dsound', required: true)
    endif
endif

# Include OpenSSL when option is true
if get_option('https') == true
    dep_openssl = dependency('openssl', required: false, static: is_static)
    if dep_openssl.found()
        mkxp_dependencies += dep_openssl
        mkxp_cflags += '-DMKXPZ_SSL'
    else
        warning('Could not locate OpenSSL library; HTTPS support remains disabled')
    endif
endif

# Win32 API: Required for GetUserNameEx
if host_system == 'windows'
    mkxp_dependencies += cc.find_library('Secur32', required: true)
endif

mkxp_includes += include_directories(
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

mkxp_sources += files(
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

if mkxp_steam
    mkxp_includes += include_directories(
        'steam'
    )

    mkxp_sources += files(
        'steam/steam.cpp'
    )
endif

if host_system == 'linux'
    mkxp_sources += files(
        'util/xdg-user-dirs.cpp',
        'oneshot/gnome-fun.cpp',
        'oneshot/xfconf-fun.cpp'
    )
endif
