proj = initramfs-data
datadir = /usr/share

install:
	mkdir -p -- $(DESTDIR)/$(datadir)/$(proj)
	cp -a -- data tools $(DESTDIR)/$(datadir)/$(proj)/
