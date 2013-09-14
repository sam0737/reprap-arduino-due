##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -std=gnu99 -Werror
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = yes
endif

#
# Build global options
##############################################################################

# Chibios/RT Chained Makefile
include $(CHIBIOS)/boards/ARDUINO_DUE/board.mk
include $(CHIBIOS)/os/hal/platforms/SAM3X8E/platform.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/SAM3X/port.mk

# Define linker script file here
LDSCRIPT= $(PORTLD)/sam3x8e_flash.ld

# List of all the board related files.
RADBOARDSRC = \
	${RAD}/radboards/RADS/radboard.c \
	${RAD}/radboards/RADS/usbcfg.c

# Required include directories
RADBOARDINC = ${RAD}/radboards/RADS
