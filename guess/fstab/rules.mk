guess-fstab:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/fstab/action

guess: guess-fstab
