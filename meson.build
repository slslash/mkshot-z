project(
	'mkshot-z',
	['cpp', 'c'],
	version: '0.0.0',
	license: 'GPL-2.0 or later',
	meson_version: '>=1.7.0',
	default_options: [
		'cpp_std=c++14',
		'c_std=c11',
		'warning_level=1',
		'buildtype=release',
		'b_ndebug=if-release'
	]
)

if host_machine.system() == 'darwin'
	error('mkshot-z doesn\'t support building on macOS with Meson. Use the Xcode project in \'macos/\' directory instead.')
endif

fs = import('fs')

is_crossbuild = meson.is_cross_build() # TODO
host_system   = host_machine.system()
host_cpu      = host_machine.cpu()
host_arch     = host_machine.cpu_family()
cc            = meson.get_compiler('c')
cxx           = meson.get_compiler('cpp')
is_debug      = get_option('buildtype').startswith('debug')
is_64bit      = host_arch in ['x86_64', 'ia64', 'aarch64', 'ppc64', 'riscv64', 'mips64', 'wasm64', 'loongarch64']

version_split = meson.project_version().split('.')
version_major = version_split[0]
version_minor = version_split[1]
version_patch = version_split[2]

opt_static              = get_option('static')
opt_force_32bit	        = get_option('force_32bit')
opt_workdir_current     = get_option('workdir_current')
opt_gles	        = get_option('gles')
opt_require_nanosleep   = get_option('require_nanosleep')
opt_appimagetool        = get_option('appimagetool')
opt_cjk_font	        = get_option('cjk_font')
opt_fluidsynth	        = get_option('fluidsynth')
opt_miniffi	        = get_option('miniffi')
opt_https	        = get_option('https')
opt_cxx11_experimental	= get_option('cxx11_experimental')
opt_steamworks_path	= get_option('steamworks_path')
opt_steam_appid		= get_option('steam_appid')
opt_steamshim_debug	= get_option('steamshim_debug')
opt_mri_includes	= get_option('mri_includes')
opt_mri_libname		= get_option('mri_libname')
opt_mri_libpath		= get_option('mri_libpath')

# Get XXD program for embedding assets and shaders into executable
xxd = find_program('xxd', native: true, required: true)
xxd_gen = generator(
    xxd,
    arguments: ['-i', '@INPUT@', '@OUTPUT@'],
    output: '@PLAINNAME@.xxd'
)

# Define global variables for mkshot-z target
mkxpz_name = 'oneshot'
mkxpz_sources = []
mkxpz_includes = []
mkxpz_dependencies = []
mkxpz_cflags = []
mkxpz_ldflags = []
mkxpz_gplv3 = false # OpenSSL is licensed under 'Apache 2.0'.

if opt_force_32bit
    mkxpz_cflags += '-m32'
endif

# Set MKXPZ_DEBUG define if we building in Debug build mode
if is_debug
    mkxpz_cflags += '-DMKXPZ_DEBUG'
endif

mkxpz_cflags += [
    '-DMKXPZ_ALCDEVICE=ALCdevice' # OpenAL
    '-DMKXPZ_BUILD_MESON',
    '-DMKXPZ_VERSION="@0@"'.format(meson.project_version()),
    '-DMKXPZ_INIT_GL_LATER'
]

if opt_require_nanosleep.auto() and host_system != 'windows' \
or opt_require_nanosleep.enabled()
    mkxpz_cflags += '-DMKXPZ_NANOSLEEP'
endif

# Suppress compiler warnings
# TODO: Reduce no-warning flags and fix warnings in code sources
warn_args = [
    '-Wno-reorder',
    '-Wno-uninitialized',
    '-Wno-unknown-pragmas',
    '-Wno-non-virtual-dtor',
    '-Wno-stringop-truncation'
]

if cxx.get_id() == 'clang'
    warn_args += [
        '-Wno-undefined-var-template', 
       '-Wno-delete-non-abstract-non-virtual-dtor'
    ]
endif

if host_system == 'windows' and cxx.get_id() != 'clang'
    mkxpz_cflags += '-masm=intel'
endif

# Link statically libgcc, libstdc++, and winpthread/pthread
if is_static
    if host_system == 'windows'
        mkxpz_ldflags += [
            '-static-libgcc',
            '-static-libstdc++',
            '-Wl,-Bstatic',
            '-lgcc',
            '-lstdc++',
            '-lpthread',
            '-Wl,-Bdynamic'
        ]
    elif host_system == 'linux'
        mkxpz_ldflags += [ '-static-libgcc', '-static-libstdc++' ]
    endif
endif


if opt_https
    mkxpz_gplv3 = true
endif

if opt_workdir_current
    mkxpz_cflags += '-DWORKDIR_CURRENT'
endif

if opt_cxx11_experimental
    mkxpz_cflags += '-DMKXPZ_EXP_FS'
endif

if opt_cjk_font
    mkxpz_cflags += '-DMKXPZ_CJK_FONT'
endif

if opt_miniffi
    mkxpz_cflags += '-DMKXPZ_MINIFFI'
endif

if opt_gles
    mkxpz_cflags += '-DGLES2_HEADER'
else
    mkxpz_dependencies += dependency('GL')
endif

# Steamworks SDK
mkxpz_steam = false
if opt_steamworks_path != ''
    steamworks_bindir = ''
    steamworks_libname = 'steam_api'

    # Determine library name and path for each platform
    if host_system == 'windows' and is_64bit
      steamworks_bindir = 'win64'
      steamworks_libname += '64'
    elif host_system == 'linux'
        if is_64bit
            steamworks_bindir = 'linux64'
        else
            steamworks_bindir = 'linux32'
        endif
    else
        error('This OS isn\'t supported by Steamworks SDK, consider disabling this option.')
    endif

    steamworks_inc = include_directories(opt_steamworks_path / 'public')
    steamworks_libpath = opt_steamworks_path / 'redistributable_bin' / steamworks_bindir
    steamworks_lib = cc.find_library(
        steamworks_libname,
        dirs: [steamworks_libpath],
        required: false
    )

    if steamworks_lib.found()
        mkxpz_steam = true

        # Append with Steamshim sources
        mkxpz_sources += files('steamshim/steamshim_child.c')
        mkxpz_includes += include_directories('steamshim')
        mkxpz_cflags += '-DMKXPZ_STEAM'
    else
        error('Could not locate Steamworks SDK in @0@'.format(steam_libpath))
    endif
endif

# Build Steamshim if we have Steamworks SDK
if mkxpz_steam
    # Define global variables for Steamshim target
    steamshim_name = 'steamshim'
    steamshim_sources = [ files('steamshim/steamshim_parent.cpp') ]
    steamshim_dependencies = [ steamworks_lib ]
    steamshim_includes = [ steamworks_inc ]
    steamshim_cflags = [ '-DGAME_LAUNCH_NAME="@0@"'.format(mkxpz_name) ]
    steamshim_ldflags = []
    steamshim_win_subsystem = 'windows,6.0'

    if opt_static
        if host_system == 'windows'
            steamshim_ldflags += [ '-static' ]
        else
            steamshim_ldflags += [ '-static-libgcc', '-static-libstdc++' ]
        endif
    endif

    if opt_steam_appid != ''
        steamshim_cflags += '-DSTEAM_APPID=@0@'.format(opt_steam_appid)
    endif

    # Set debug define and Console subsystem version for Steamshim
    if opt_steamshim_debug
        steamshim_cflags += '-DSTEAMSHIM_DEBUG'
        steamshim_win_subsystem = 'console,6.0'
    endif
endif

# Include subdirectories
subdir('assets')
subdir('shader')
subdir('src')
subdir('binding')

if host_system == 'windows'
    subdir('windows')
else
    subdir('posix')
endif

# Steamshim executable target
if mkxpz_steam
    executable(
        steamshim_name,
        sources: steamshim_sources,
        include_directories: steamshim_includes,
        dependencies: steamshim_dependencies,
        c_args: steamshim_cflags,
        cpp_args: steamshim_cflags,
        link_args: steamshim_ldflags,
        install: (appimagetool == ''),
        win_subsystem: steamshim_win_subsystem
    )
endif

# ModShot executable target
executable(
    mkxpz_name,
    sources: mkxpz_sources,
    include_directories: mkxpz_includes,
    dependencies: mkxpz_dependencies,
    c_args: mkxpz_cflags,
    cpp_args: mkxpz_cflags + warn_args,
    link_args: mkxpz_ldflags,
    install: (opt_appimagetool == ''),
    win_subsystem: 'windows,6.0'
)
