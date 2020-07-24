.PHONY: guess-qemu

guess-qemu:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/qemu/action

guess: guess-qemu
