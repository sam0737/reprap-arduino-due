##############################################################################
# RAD Hardware Configuration
#
# Uncomment/comment the following according to your hardware

# -----------------------------------------------------
# Radboard
# -----------------------------------------------------
# -- RADS / Reprap Arduino Due Shield 
include $(RAD)/radboards/RADS/radboard.mk
#include $(RAD)/radboards/Test/radboard.mk

# -----------------------------------------------------
# Machine
# -----------------------------------------------------
# -- Generic machine template
include $(RAD)/machines/generic/machine.mk

# -----------------------------------------------------
# Auxiliary
# -----------------------------------------------------
# == Display
# -- ST7565 128x64 dot-matrix
# include $(RAD)/displays/ST7565/display.mk
# -- HD44780 20x4 character-based
# include $(RAD)/displays/HD44780/display.mk
# == Fat
# include rad_storage.mk
# == MSD
# include rad_msd.mk