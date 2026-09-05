CFLAGS += -Wall -D_GNU_SOURCE -g

ifeq ($(origin PKG_CONFIG), undefined)
  PKG_CONFIG = pkg-config
  ifeq ($(shell which $(PKG_CONFIG) 2>/dev/null),)
    $(error $(PKG_CONFIG) not found)
  endif
endif

CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)

SOURCES_HELPER = libolsrdhelper.c neigh.c uclient.c
FILES_HELPER = $(SOURCES_HELPER) libolsrdhelper.h uclient.h

SOURCES_LUA = lua-olsrd.c
FILES_LUA = $(SOURCES_LUA)

all: olsrd.so libolsrdhelper.so

libolsrdhelper.so: $(FILES_HELPER)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $(SOURCES_HELPER) $(LDLIBS) -lgluonutil -luclient

olsrd.so: libolsrdhelper.so $(FILES_LUA)
	$(CC) $(LUA_CFLAGS) $(CFLAGS) $(LDFLAGS) -L. -shared -fPIC -o $@ $(SOURCES_LUA) $(LDLIBS) -lgluonutil -lolsrdhelper -llua-jsonc

clean:
	rm -f *.so *.o
