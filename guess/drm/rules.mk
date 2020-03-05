guess-drm:
	$V echo "Processing $@ ..."
	@ GUESS_SUFFIX=add:$@ \
	    $(DETECTDIR)/drm/action

guess: guess-drm
