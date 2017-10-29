INSTALLDIR = $(N64_INST)
CFLAGS = -std=gnu99 -O2 -Wall -I../../include
LDFLAGS = -lpng

all: mksprite convtool

mksprite: mksprite.o
	$(CC) $< -o $@ $(LDFLAGS)

%o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

convtool: convtool.c

install: mksprite convtool
	install -m 0755 mksprite $(INSTALLDIR)/bin
	install -m 0755 convtool $(INSTALLDIR)/bin

.PHONY: clean install

clean:
	rm -rf mksprite
	rm -rf convtool
