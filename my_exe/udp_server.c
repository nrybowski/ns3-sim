#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <time.h>

int main(int argc, char **argv) {

    struct timespec tp;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1);
    struct sockaddr_in dst;

    memset(&dst, 0, sizeof(struct sockaddr_in));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(8000);
    dst.sin_addr.s_addr = inet_addr("0.0.0.0");
    
    int ret = bind(fd, (struct sockaddr*) &dst, sizeof(dst));
    if (ret == -1) {
	perror("error");
	close(fd);
	exit(1);
    }

    ssize_t recvd;
    char buf[1024];
    while (1) {
	recvd = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
	clock_gettime(CLOCK_REALTIME, &tp);
	printf(buf);
    }

    close(fd);
    return 0;
}
