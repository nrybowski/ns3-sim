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


    /*struct ifaddrs *ifap;
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
    freeifaddrs(ifap);*/

    struct timespec tp;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1);

    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(struct sockaddr_in));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(8000);
    dst.sin_addr.s_addr = inet_addr("0.0.0.0");
    
    if (bind(fd, (struct sockaddr*) &dst, sizeof(dst)) == -1) {
	perror("error");
	close(fd);
	exit(1);
    }

    ssize_t recvd;
    time_t received;
    uint32_t buf[2];

    struct sockaddr_in src;
    socklen_t addrlen = sizeof(src);

    FILE *log_fd = fopen("/var/log/udp_ping.log", "w");
    if (log_fd == NULL) {
	    perror("udp log file");
	    exit(1);
    }

    my data;
    char *rid = NULL;
    char *raddr = NULL;
    while (1) {
	memset(&src, 0, sizeof(src));
	memset(&data, 0, sizeof(data));
	//recvd = recvfrom(fd, &buf, sizeof(buf), 0, (struct sockaddr*) &src, &addrlen);
	recvd = recvfrom(fd, &data, sizeof(data), 0, (struct sockaddr*) &src, &addrlen);
	if (recvd == -1) {
	    perror("error");
	    continue;
	}

	clock_gettime(CLOCK_REALTIME, &tp);
	//received = (((time_t) ntohl(buf[0]) << 32) & 0xffffffff00000000) | ((time_t) ntohl(buf[1]) & 0x00000000ffffffff);
	//fprintf(log_fd, "%s, %s, %lu, %lu\n", inet_ntoa(data.src), inet_ntoa(src.sin_addr), tp.tv_sec * 1000000 + tp.tv_nsec / 1000, data.current);
	//raddr = inet_ntoa(src.sin_addr);
	rid = inet_ntoa(data.src);
	fprintf(log_fd, "%s, %lu, %lu\n", rid, data.current, tp.tv_sec * 1000000 + tp.tv_nsec / 1000);
    }

    fclose(log_fd);
    close(fd);
    return 0;
}
