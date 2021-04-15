# standard compliance
CFLAGS         += -std=c99

# warnings
CFLAGS         += -Wall -Wextra -Wformat=2 -Wshadow -Wpointer-arith
CFLAGS         += -pedantic

all: respondd.so

%.c: %.h

respondd.so: netlink.c ifaces.c airtime.c clients.c neighbours.c respondd.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -fPIC -D_GNU_SOURCE -lnl-tiny -o $@ $^ $(LDLIBS)

clean:
	rm -rf *.so
