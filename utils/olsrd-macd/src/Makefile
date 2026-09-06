CFLAGS += -Wall -D_GNU_SOURCE

PKG_CONFIG ?= pkg-config

CFLAGS += $(shell $(PKG_CONFIG) --cflags json-c)
LDLIBS += $(shell $(PKG_CONFIG) --libs json-c) -lubox

all: olsr-macd

olsr-macd: olsr-macd.c olsr-macd.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -fPIE -o $@ olsr-macd.c $(LDLIBS)

clean:
	rm -f olsr-macd
