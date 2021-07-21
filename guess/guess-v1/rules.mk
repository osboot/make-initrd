.PHONY: guess-qemu

guess-v1:
	$V echo "Processing $@ ..."
	@$(DETECTDIR)/$@/action

guess: guess-v1
