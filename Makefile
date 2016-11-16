#directory in which is placed "netlink" folder with header files
path_to_netlink_header = /usr/include/libnl3/

TARGET = accumulation-rssi
PREFIX = /usr/local

CC = gcc
CFLAGS = -Wall -I$(path_to_netlink_header)
LDLIBS = -lm -lnl-3 -lnl-genl-3

SCRS =  main.c \
	accumulation.c arguments.c \
	iw_nl80211.c utils.c
OBJS = $(SCRS:.c=.o)

.PHONY: all clean install uninstall

all:		$(TARGET)

$(TARGET):	$(OBJS)
		$(CC) $(CFLAGS) $(LDLIBS) $(OBJS) -o $(TARGET)
		
.c.o:
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -rf $(TARGET) $(OBJS)

install:
		install $(TARGET) $(PREFIX)/bin
	
uninstall:
		rm -rf $(PREFIX)/bin/$(TARGET)
