all: libpacketmark.so

libpacketmark.so: libpacketmark.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -Wall -shared -fPIC -o $@ $^ -ldl $(LDLIBS)
