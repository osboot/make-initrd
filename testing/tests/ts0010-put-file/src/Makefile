CPPFLAGS = -I.
CFLAGS = -g -O0 -pedantic -Wall -Wextra

all: foo

libfoo.o: libfoo.c
	$(COMPILE.c) -fPIC -shared -o $@ $^

libfoo.so: libfoo.o
	$(LINK.o) -shared -fvisibility=default -o $@ $^

foo.o: foo.c
	$(COMPILE.c) -o $@ $<

foo: foo.o
	$(LINK.o) -o $@ $< -L. -lfoo

foo: libfoo.so

clean:
	@$(RM) -f -- foo *.o *.so
