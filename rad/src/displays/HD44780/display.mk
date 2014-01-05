# uGFX Chained Makefile
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/drivers/tdisp/HD44780/tdisp_lld.mk

UDEFS += -DHAL_USE_GFX=TRUE

# List of all the display related files.
DISPLAYSRC = \

# Required include directories
DISPLAYINC = ${RAD}/displays/HD44780
