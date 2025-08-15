SET(SOURCES ui_Ladeanzeige.c
    ui_Tacho.c
    ui_Settings.c
    ui_WiFiInfo.c
    ui_SRInfo.c
    ui_theme_manager.c
    ui_themes.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_img_secondride_logows_bar_png.c
    ui_img_secondride_logosw_png.c
    ui_img_1767220752.c
    ui_img_602373867.c
    ui_img_133863867.c
    ui_img_1435283132.c
    ui_img_400979676.c
    ui_img_517647767.c
    ui_img_secondride_logows_bar_klein_png.c
    ui_img_secondride_logows_klein_png.c
    ui_img_tachoskala_png.c
    ui_img_zeiger2_png.c
    ui_img_strich_kurz_png.c
    ui_img_374202920.c
    ui_img_798314644.c
    ui_img_1045174431.c
    ui_img_1569223354.c
    ui_img_secondride_diagnose_png.c
    fonts/ui_font_ArialBig.c
    fonts/ui_font_ArialMed.c)

add_library(ui ${SOURCES})
