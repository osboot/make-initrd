.PHONY: guess-lkrg

guess-lkrg:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/lkrg/action

guess: guess-lkrg
