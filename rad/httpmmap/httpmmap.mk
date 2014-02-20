# don't re-define GFXLIB if it has been set elsewhere, e.g in Makefile
ifeq ($(HTTPMMAP),)
	$(error Please define the variable HTTPMMAP in the Makefile)
endif

HTTPMMAPINC +=  $(HTTPMMAP)/include $(HTTPMMAP)/Win32 $(HTTPMMAP)/Win32/include
HTTPMMAPSRC +=	$(HTTPMMAP)/src/httpmmap.c $(HTTPMMAP)/Win32/httpmmap_lld.c

ULIBDIR += $(HTTPMMAP)/Win32/lib
ULIBS += -llibmicrohttpd
UDEFS += -DHAS_HTTPMMAP

MAKE_ALL_RULE_HOOK:
	cp $(HTTPMMAP)/Win32/bin/*.dll $(BUILDDIR)