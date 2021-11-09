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
    char buf[1024];

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1);
    struct sockaddr_in dst;

    memset(&dst, 0, sizeof(struct sockaddr_in));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(8000);
    dst.sin_addr.s_addr = inet_addr("10.0.0.71");

    while (1) {
	memset(&buf, 0, sizeof(buf));
	clock_gettime(CLOCK_REALTIME, &tp);
	snprintf(buf, sizeof(buf), "%lu", tp.tv_sec * 1000000 + tp.tv_nsec / 1000);
	printf("%s\n", buf);

	if (-1 == sendto(fd, buf, strlen(buf), 0, (struct sockaddr*) &dst, sizeof(dst))) {
	    perror("error");
	    close(fd);
	    exit(1);
	}
	usleep(5000);
    }
    close(fd);
    return 0;
}
