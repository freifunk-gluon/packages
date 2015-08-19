all: uradvd

uradvd: uradvd.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -Wall -o $@ $^ $(LDLIBS) -lrt
