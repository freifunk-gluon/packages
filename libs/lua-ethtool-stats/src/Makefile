all: ethtool_stats.so

CFLAGS += -Wall

ethtool_stats.so: ethtool_stats.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -fPIC -o $@ $^ $(LDLIBS)
