CC	?= cc
STRIP	?= strip

OPTS	:= -O2
CFLAGS	+= -Isrc -Wall -Wextra -s -ffunction-sections -fdata-sections -fno-strict-aliasing
LDFLAGS	+= -Wl,--gc-sections -lusb
SFLAGS	:= -s -R .comment -R .gnu.version -R .note -R .note.ABI-tag

SOURCES	:= $(wildcard src/*.c)
OBJECTS	:= $(patsubst %.c,%.o,$(SOURCES))
TARGETS := usbtemp

all: $(OBJECTS) $(TARGETS)

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(OPTS) -c -o $@ $<

$(TARGETS): $(OBJECTS)
	$(CC) $(CFLAGS) $(OPTS) -o $@ $@.c $^ $(LDFLAGS)
	$(STRIP) $(SFLAGS) $@

clean:
	rm -fr $(OBJECTS)
	rm -f $(TARGETS)
