# Ruby
if opt_mri_includes == ''
    mkxpz_cflags += '-fdeclspec'
    mkxpz_dependencies += dependency('ruby-3.4')
else
    mkxpz_cflags += '-I@0@'.format(opt_mri_includes)
    mkxpz_dependencies += cc.find_library(opt_mri_libname, dirs: [opt_mri_libpath])
endif

mkxpz_binding_sources = files(
    'module_rpg.cpp',
    'binding-mri.cpp',
    'binding-util.cpp',
    'etc-binding.cpp',
    'table-binding.cpp',
    'graphics-binding.cpp',
    'input-binding.cpp',
    'audio-binding.cpp',
    'bitmap-binding.cpp',
    'sprite-binding.cpp',
    'plane-binding.cpp',
    'viewport-binding.cpp',
    'font-binding.cpp',
    'window-binding.cpp',
    'windowvx-binding.cpp',
    'tilemap-binding.cpp',
    'tilemapvx-binding.cpp',
    'filesystem-binding.cpp',
    'http-binding.cpp',
    'steam-binding.cpp',

    'oneshot/oneshot-binding.cpp',
    'oneshot/journal-binding.cpp',
    'oneshot/niko-binding.cpp',
    'oneshot/wallpaper-binding.cpp'
)

#if mkxpz_steam
#    mkxpz_binding_sources += files(
#        'cusl-binding.cpp'
#    )
#endif

if opt_miniffi
    mkxpz_binding_sources += files('miniffi.cpp', 'miniffi-binding.cpp')
endif

mkxpz_sources += mkxpz_binding_sources
mkxpz_includes += include_directories('.')
