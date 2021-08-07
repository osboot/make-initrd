.PHONY: guess-smart-card

guess-smart-card:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/smart-card/action

guess: guess-smart-card
