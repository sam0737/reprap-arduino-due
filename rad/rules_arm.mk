include $(CHIBIOS)/os/ports/GCC/ARMCMx/rules.mk

MAKE_ALL_RULE_HOOK: $(BUILDDIR)/$(PROJECT).lst $(BUILDDIR)/$(PROJECT).siz 

%.lst: %.elf
	@echo 'Creating listing: $@'
	@$(OD) -h -S $< > $@
	@echo ' '
	
%.siz: %.elf
	@echo 'Output Size:'
	@$(SIZE) --format=berkeley -t $<
	@echo ' '
