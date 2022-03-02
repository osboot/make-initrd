ifneq ($(LOCALES),)

define enforce_utf8
$(basename $(1)).UTF-8
endef

LOCALES_LIST = $(sort $(foreach locale,$(LOCALES),$(call enforce_utf8,$(locale))))
LANG = $(call enforce_utf8,$(firstword $(LOCALES)))

$(call set-sysconfig,init,LANG,$(LANG))

.PHONY: locales

locales: create
	@$(VMSG) "Adding locales and l10n..."
	@for locale in $(LOCALES_LIST); do \
	    $(FEATURESDIR)/locales/bin/put-locale "$$locale"; \
	    [ -z "$(LOCALE_TEXTDOMAIN)" ] || $(FEATURESDIR)/locales/bin/put-l10n "$$locale" $(LOCALE_TEXTDOMAIN); \
	done

pack: locales

endif
