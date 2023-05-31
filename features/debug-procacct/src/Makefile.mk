procacct_DEST = $(dest_data_bindir)/procacct
procacct_SRCS = $(FEATURESDIR)/debug-procacct/src/procacct.c
procacct_CFLAGS = -D_GNU_SOURCE -Idatasrc/libinitramfs -DPROCACCT_BPF_FILE=\"/bin/procacct-bpf.o\"
procacct_LIBS = -L$(dest_data_libdir) -linitramfs -lbpf

PROGS += procacct

procacct_bpf_DEST = $(dest_data_bindir)/procacct-bpf.o
procacct_bpf_SRCS = $(FEATURESDIR)/debug-procacct/src/procacct-bpf.c

CLANG ?= clang-15
LLVM_STRIP = llvm-strip

$(dest_data_bindir)/procacct-bpf.o: $(procacct_bpf_SRCS)
	$(Q)mkdir -p -- $(dir $@)
	$(call quiet_cmd,BPF,$@,$(CLANG)) -target bpf -g -O2 -Wall -Wextra -o $@ -c $<
	$(call quiet_cmd,STRIP,$@,$(LLVM_STRIP)) -g $@

build-progs: $(dest_data_bindir)/procacct-bpf.o
