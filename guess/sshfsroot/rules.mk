.PHONY: guess-sshfsroot

guess-sshfsroot:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/sshfsroot/action

guess: guess-sshfsroot
