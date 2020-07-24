.PHONY: guess-device

guess-device:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/device/action

guess: guess-device
