
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
