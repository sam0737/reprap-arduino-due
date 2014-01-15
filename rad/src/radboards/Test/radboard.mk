##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -std=gnu99 -Werror
endif

TRGT = 
CC   = $(TRGT)gcc
AS   = $(TRGT)gcc -x assembler-with-cpp
LD   = $(TRGT)gcc

# List all default C defines here, like -D_DEBUG=1
DDEFS = -DSIMULATOR -DSHELL_USE_IPRINTF=FALSE

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lws2_32

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = yes
endif

#
# Build global options
##############################################################################

# Chibios/RT Chained Makefile
include $(CHIBIOS)/boards/simulator/board.mk
include ${CHIBIOS}/os/hal/platforms/Win32/platform.mk
include ${CHIBIOS}/os/ports/GCC/SIMIA32/port.mk

# List of all the board related files.
RADBOARDSRC = \
	${RAD}/radboards/Test/radboard.c

# Required include directories
RADBOARDINC = ${RAD}/radboards/Test

RAD_RULES = rules_win32.mk