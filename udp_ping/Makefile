CFLAGS=-fPIC -g
LDFLAGS=-rdynamic -pie

all: udp_client udp_server

udp_client:
	gcc $(CFLAGS) $(LDFLAGS) -o udp_client udp_client.c
udp_server:
	gcc $(CFLAGS) $(LDFLAGS) -o udp_server udp_server.c
