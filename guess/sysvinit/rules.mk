guess-sysvinit:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/sysvinit/action

guess: guess-sysvinit
