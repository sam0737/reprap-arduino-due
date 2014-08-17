# uGFX Chained Makefile
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/drivers/multiple/Win32/gdisp_lld.mk

UDEFS += -DHAL_USE_GFX=TRUE -DGINPUT_NEED_MOUSE=TRUE

# List of all the display related files.
DISPLAYSRC = \

# Required include directories
DISPLAYINC = ${RAD}/displays/Win32QVGA ${RAD}/ui/viewmodel/qvga
