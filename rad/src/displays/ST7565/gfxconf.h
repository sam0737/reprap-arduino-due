#ifndef _GFXCONF_H
#define _GFXCONF_H

#define RAD_DISPLAY_ST7565          TRUE

/* The operating system to use - one of these must be defined */
#define GFX_USE_OS_CHIBIOS          TRUE

/* GFX sub-systems to turn on */
#define GFX_USE_GDISP               TRUE

/* Features for the GDISP subsystem.  */
#define GDISP_USE_CUSTOM_BOARD             TRUE
#define GDISP_NEED_VALIDATION         TRUE
#define GDISP_NEED_CLIP     TRUE
#define GDISP_NEED_TEXT     TRUE
#define GDISP_NEED_CIRCLE   TRUE
#define GDISP_NEED_ELLIPSE    TRUE
#define GDISP_NEED_ARC      TRUE
#define GDISP_NEED_SCROLL   FALSE
#define GDISP_NEED_PIXELREAD          FALSE
#define GDISP_NEED_CONTROL    TRUE
#define GDISP_NEED_MULTITHREAD        FALSE
#define GDISP_NEED_ASYNC    FALSE
#define GDISP_NEED_MSGAPI   FALSE

#define GDISP_INCLUDE_USER_FONTS          TRUE
#define GDISP_INCLUDE_FONT_DEJAVUSANS12     TRUE

#endif /* _GFXCONF_H */
