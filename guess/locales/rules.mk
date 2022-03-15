.PHONY: guess-locales

guess-locales:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/locales/action

guess: guess-locales
