.PHONY: guess-rdshell

guess-rdshell:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/rdshell/action

guess: guess-rdshell
