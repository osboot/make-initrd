.PHONY: guess-keyboard

guess-keyboard:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=rescue:$@ \
	    $(DETECTDIR)/keyboard/action

guess: guess-keyboard
