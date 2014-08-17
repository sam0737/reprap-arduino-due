# uGFX Chained Makefile
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/drivers/tdisp/VirtualText/tdisp_lld.mk

UDEFS += -DHAL_USE_GFX=TRUE

# List of all the display related files.
DISPLAYSRC = \

# Required include directories
DISPLAYINC = ${RAD}/displays/Virtual20x4 ${RAD}/ui/viewmodel/text
