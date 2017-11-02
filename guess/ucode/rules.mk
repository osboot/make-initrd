guess-ucode:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/ucode/action

guess: guess-ucode
