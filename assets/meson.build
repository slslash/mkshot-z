mkxpz_assets = files('icon.png', 'gamecontrollerdb.txt')

if opt_cjk_font
    mkxpz_assets += files('wqymicrohei.ttf')
else
    mkxpz_assets += files('liberation.ttf')
endif

mkxpz_assets_xxd = xxd_gen.process(mkxpz_assets)

mkxpz_sources += mkxpz_assets_xxd
