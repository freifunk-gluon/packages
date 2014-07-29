all: libpacketmark.so

libpacketmark.so: libpacketmark.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -Wall -nostartfiles -shared -fPIC -o $@ $^ -ldl $(LDLIBS)
