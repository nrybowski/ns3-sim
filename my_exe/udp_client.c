#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include "udp.h"

int main(int argc, char **argv) {
    struct ifaddrs *ifap;
    if (getifaddrs(&ifap) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    char *rid = NULL;
    struct ifaddrs *iface = NULL;
    for (iface = ifap; iface != NULL; iface = iface->ifa_next) {
	if (iface->ifa_addr == NULL)
	    continue;

        if (strncmp("lo", iface->ifa_name, 2) == 0 && iface->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *addr = (struct sockaddr_in*) iface->ifa_addr;
	    rid = inet_ntoa(addr->sin_addr);
	    if (strncmp("127.0.0.1", rid, 9) != 0) {
            	printf("%s %s\n", iface->ifa_name, rid);
		break;
	    }
        }
    }
    freeifaddrs(ifap);

    my data;
    inet_aton(rid, &data.src);

    struct timespec tp;
    //char buf[1024];
    time_t current;
    int32_t buf[2];

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1);
    struct sockaddr_in dst;

    memset(&dst, 0, sizeof(struct sockaddr_in));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(8000);
    dst.sin_addr.s_addr = inet_addr("10.0.0.10");

    while (1) {
	memset(&buf, 0, sizeof(buf));
	clock_gettime(CLOCK_REALTIME, &tp);
	data.current = tp.tv_sec * 1000000 + tp.tv_nsec / 1000;
	//buf[0] = htonl((current >> 32) & 0xffffffff);
	//buf[1] = htonl(current & 0xffffffff);

	//snprintf(buf, sizeof(buf), "%lu", tp.tv_sec * 1000000 + tp.tv_nsec / 1000);
	//printf("%s\n", buf);
	printf("%lu\n", data.current);

	//if (-1 == sendto(fd, buf, strlen(buf), 0, (struct sockaddr*) &dst, sizeof(dst))) {
	//if (-1 == sendto(fd, &buf, sizeof(buf), 0, (struct sockaddr*) &dst, sizeof(dst))) {
	if (-1 == sendto(fd, &data, sizeof(data), 0, (struct sockaddr*) &dst, sizeof(dst))) {
	    perror("error");
	    close(fd);
	    exit(1);
	}
	usleep(5000);
    }
    close(fd);
    return 0;
}
