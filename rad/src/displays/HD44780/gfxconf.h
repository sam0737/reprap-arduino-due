#ifndef _GFXCONF_H
#define _GFXCONF_H

#define RAD_DISPLAY_HD44780         TRUE

/* The operating system to use - one of these must be defined */
#define GFX_USE_OS_CHIBIOS          TRUE

/* GFX sub-systems to turn on */
#define GFX_USE_TDISP               TRUE

/* Features for the TDISP subsystem.  */
/* Set this to TRUE if need a custom board file.
 * The name of your board file must be "tdisp-lld-board.h" */
#define TDISP_USE_CUSTOM_BOARD      TRUE
/* Set to TRUE if multithreads need to read or
 * write to the display. If not, set this to FALSE */
#define TDISP_NEED_MULTITHREAD      FALSE
/* If you use the busy flag or you want to read
 * from the display, set this to TRUE otherwise
 * leaf it to FALSE. Most users do not use the
 * read function of the display */
#define TDISP_NEED_READ             FALSE
/* Number of colums of the connected display */
#define TDISP_COLUMNS               20
/* Number of rows of the connect display */
#define TDISP_ROWS                  4
/* Use dimming of backlight */
#define TDISP_USE_BACKLIGHT         FALSE

#endif /* _GFXCONF_H */
