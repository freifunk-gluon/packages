all: platform_info.so

CFLAGS += -Wall

platform_info.so: platform_info.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $^ $(LDLIBS) -lplatforminfo
