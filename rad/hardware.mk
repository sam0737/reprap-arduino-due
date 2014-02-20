##############################################################################
# RAD Hardware Configuration
#
# Uncomment/comment the following according to your hardware

# -----------------------------------------------------
# Radboard
# -----------------------------------------------------
# -- RADS / Reprap Arduino Due Shield 
ifneq ($(RAD_TEST),) 
  include $(RAD)/radboards/Test/radboard.mk
  include $(HTTPMMAP)/httpmmap.mk
else
  include $(RAD)/radboards/RADS/radboard.mk
endif

# -----------------------------------------------------
# Machine
# -----------------------------------------------------
# -- Generic machine template
include $(RAD)/machines/generic/machine.mk

# -----------------------------------------------------
# Auxiliary
# -----------------------------------------------------

# ==== Storage (Must choose one) ====
# -- FatFS (Embedded System)
# include rad_storage_fatfs.mk
# -- Win32 Emulation
include rad_storage_Win32.mk
# -- Dummy (Disable Storage)
# include rad_storage_dummy.mk

# ==== Display ====
# -- ST7565 128x64 dot-matrix
# include $(RAD)/displays/ST7565/display.mk
# -- HD44780 20x4 character-based
# include $(RAD)/displays/HD44780/display.mk
# -- Virtual - 20x4 character-based
include $(RAD)/displays/Virtual20x4/display.mk
# -- Virtual QVGA (Win32)
# include $(RAD)/displays/Win32QVGA/display.mk

# ==== USB Mass Storage Device ====
# include rad_msd.mk