guess-syslog:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/syslog/action

guess: guess-syslog
