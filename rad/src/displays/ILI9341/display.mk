# uGFX Chained Makefile
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/drivers/gdisp/ILI9341/gdisp_lld.mk

UDEFS += -DHAL_USE_GFX=TRUE

# List of all the display related files.
DISPLAYSRC = \

# Required include directories
DISPLAYINC = ${RAD}/displays/ILI9341 ${RAD}/ui/viewmodel/qvga
