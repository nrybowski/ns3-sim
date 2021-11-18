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
    //uint32_t buf[2];

    struct sockaddr_in src;
    socklen_t addrlen = sizeof(src);

    FILE *log_fd = fopen("/var/log/udp_ping.log", "w");
    if (log_fd == NULL) {
	    perror("udp log file");
	    exit(1);
    }

    //my data;
    char *raddr = NULL;
    long buf;
    while (1) {
	memset(&src, 0, sizeof(src));
	//memset(&data, 0, sizeof(data));
	recvd = recvfrom(fd, &buf, sizeof(buf), 0, (struct sockaddr*) &src, &addrlen);
	//recvd = recvfrom(fd, &data, sizeof(data), 0, (struct sockaddr*) &src, &addrlen);
	if (recvd == -1) {
	    perror("error");
	    continue;
	}

	clock_gettime(CLOCK_REALTIME, &tp);
	//received = (((time_t) ntohl(buf[0]) << 32) & 0xffffffff00000000) | ((time_t) ntohl(buf[1]) & 0x00000000ffffffff);
	//fprintf(log_fd, "%s, %s, %lu, %lu\n", inet_ntoa(data.src), inet_ntoa(src.sin_addr), tp.tv_sec * 1000000 + tp.tv_nsec / 1000, data.current);
	raddr = inet_ntoa(src.sin_addr);
	//fprintf(log_fd, "%s, %lu, %lu\n", raddr, data.current, tp.tv_sec * 1000000 + tp.tv_nsec / 1000);
	fprintf(log_fd, "%s, %lu, %lu\n", raddr, buf, tp.tv_sec * 1000000 + tp.tv_nsec / 1000);
    }

    fclose(log_fd);
    close(fd);
    return 0;
}
