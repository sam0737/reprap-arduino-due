# uGFX Chained Makefile
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/drivers/gdisp/st7565/gdisp_lld.mk

UDEFS += -DHAL_USE_GFX=TRUE

# List of all the display related files.
DISPLAYSRC = \

# Required include directories
DISPLAYINC = ${RAD}/displays/ST7565
