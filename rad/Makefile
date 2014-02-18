##############################################################################
# Architecture or project specific options
#

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = rad

# Imported source files and paths
CHIBIOS = chibios
GFXLIB = ugfx
HTTPMMAP = httpmmap
RAD = src

# Chibios/RT Chained Makefile
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/kernel/kernel.mk

# Additional library
include format/format.mk

# RAD Chained Makefile
include hardware.mk

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(TESTSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(FATFSSRC) \
       $(GFXSRC) \
       $(FORMATSRC) \
       $(RADBOARDSRC) \
       $(MACHINESRC) \
       $(DISPLAYSRC) \
       $(HTTPMMAPSRC) \
       $(CHIBIOS)/os/various/chprintf.c \
       $(CHIBIOS)/os/various/shell.c \
       $(MSDSRC) \
       $(wildcard ${RAD}/*.c)

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(PORTASM)

INCDIR = $(PORTINC) $(KERNINC) $(TESTINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) \
         $(CHIBIOS)/os/various \
         $(MSDINC) \
         $(FATFSINC) \
         $(GFXINC) \
         $(FORMATINC) \
         $(RADBOARDINC) \
         $(MACHINEINC) \
         $(DISPLAYINC) \
         $(HTTPMMAPINC) \
         src \

#
# Project, sources and paths
##############################################################################

include $(RAD_RULES)
