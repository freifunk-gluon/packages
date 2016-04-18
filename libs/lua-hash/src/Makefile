all: hash.so

CFLAGS += -Wall

hash.so: hash.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $^ $(LDLIBS) -llua -lubox
