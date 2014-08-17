#ifndef _GFXCONF_H
#define _GFXCONF_H

#define GDISP_SCREEN_HEIGHT   240
#define GDISP_SCREEN_WIDTH    320

#define RAD_DISPLAY_WIN32QVGA         TRUE

#define DISPLAY_MENU_LINE_HEIGHT      32
#define DISPLAY_MENU_ICON_WIDTH       64

/* Builtin contrast support */
#define RAD_DISPLAY_CONTRAST_SUPPORT  FALSE

/* The operating system to use - one of these must be defined */
#define GFX_USE_OS_CHIBIOS          TRUE

/* GFX sub-systems to turn on */
#define GFX_USE_GDISP               TRUE

/* Features for the GDISP subsystem.  */
#define GDISP_USE_CUSTOM_BOARD      TRUE
#define GDISP_NEED_TEXT             TRUE
#define GDISP_NEED_CIRCLE           TRUE
#define GDISP_NEED_CONTROL          TRUE
#define GDISP_NEED_MULTITHREAD      TRUE
#define GDISP_NEED_CONVEX_POLYGON   TRUE

#define GDISP_INCLUDE_FONT_DEJAVUSANS16   TRUE
#define GDISP_INCLUDE_FONT_DEJAVUSANS24   TRUE
#define GDISP_INCLUDE_FONT_DEJAVUSANS32   TRUE

#define GFX_USE_GINPUT          TRUE
#define GFX_USE_GEVENT          TRUE
#define GFX_USE_GTIMER          TRUE
#define GINPUT_NEED_MOUSE       TRUE

#endif /* _GFXCONF_H */
