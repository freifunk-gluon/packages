LOCKFILE?=/var/lib/ebtables/lock
LOCKDIR:=$(shell echo $(LOCKFILE) | sed 's/\(.*\)\/.*/\1/')/


CFLAGS += -Iinclude
CFLAGS += -Wall

EXT_TABLES:=filter nat broute
EXT_FUNC:=arp ip6 ip limit mark mark_m nat standard
EXT_OBJS:=$(foreach T,$(EXT_FUNC),extensions/ebt_$(T).o) $(foreach T,$(EXT_TABLES),extensions/ebtable_$(T).o)

OBJECTS2:=getethertype.o communication.o libebtc.o useful_functions.o ebtables.o

OBJECTS:=$(OBJECTS2) $(EXT_OBJS)

PROGSPECS:= \
	-DLOCKFILE=\"$(LOCKFILE)\" \
	-DLOCKDIR=\"$(LOCKDIR)\"

all: ebtables

%.o: %.c include/ebtables_u.h include/ethernetdb.h
	$(CC) $(CFLAGS) $(PROGSPECS) -c -o $@ $<

ebtables: ebtables-standalone.o $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f ebtables
	rm -f *.o
	rm -f extensions/*.o
