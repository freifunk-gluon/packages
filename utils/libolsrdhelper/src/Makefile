CFLAGS += -Wall -D_GNU_SOURCE

PKG_CONFIG ?= pkg-config

CFLAGS += $(shell $(PKG_CONFIG) --cflags json-c olsr-macd)
LDLIBS += $(shell $(PKG_CONFIG) --libs json-c)

SOURCES_HELPER = libolsrdhelper.c neigh.c uclient.c
FILES_HELPER = $(SOURCES_HELPER) libolsrdhelper.h uclient.h

all: libolsrdhelper.so olsrd.so

libolsrdhelper.so: $(FILES_HELPER)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $(SOURCES_HELPER) $(LDLIBS) -lubox -luclient

olsrd.so: libolsrdhelper.so lua-olsrd.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -L. -shared -fPIC -o $@ lua-olsrd.c $(LDLIBS) -lolsrdhelper -llua-jsonc

clean:
	rm -f *.so
