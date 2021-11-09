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
    clock_gettime(CLOCK_REALTIME, &tp);
    char buf[1024];
    memset(&buf, 0, sizeof(buf));
    //snprintf(buf, sizeof(buf), "%lu", tp.tv_sec+tp.tv_nsec);
    snprintf(buf, sizeof(buf), "%lu", tp.tv_sec * 1000000 + tp.tv_nsec / 1000);
    printf(buf, "\n");
    size_t len = strlen(buf);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) exit(1);
    struct sockaddr_in dst;

    memset(&dst, 0, sizeof(struct sockaddr_in));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(8000);
    dst.sin_addr.s_addr = inet_addr("10.0.0.71");
    
    int ret = sendto(fd, buf, len, 0, (struct sockaddr*) &dst, sizeof(dst));
    if (ret == -1) {
	perror("error");
	close(fd);
	exit(1);
    }
    close(fd);
    return 0;
}
