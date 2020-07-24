.PHONY: guess-net

guess-net:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/net/action

guess: guess-net
