#include<stdio.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<error.h>
#include<sys/poll.h>
int main()
{
	int sockfd = socket(IF_INET, SOCK_STREAM, 0);
	struct sockaddr_in seraddr;
	memset(&seraddr, 0, sizeof(struct sockaddr_in));
	seraddr.sin_family = IF_INET;
	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	seraddr.sin_port = htons(2048);
	if (-1 == bind(sockfd, (struct sockaddr*)&seraddr, sizeof(struct sockaddr))) {
		perror("bind");
		return -1;
	}
	listen(sockfd, 10);
	
	struct pollfd fds[1024] = { 0 };
	fds[sockfd].fd = sockfd;
	fds[sockfd].events = POLLIN;

	int maxfd = sockfd;

	while (1) {
		int nready = poll(fds, maxfd + 1, -1);
		if (fds[sockfd].revents & POLLIN) {
			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
			printf("clientfd: %d", clientfd);

			fds[clientfd].fd = clientfd;
			fds[clientfd].events = POLLIN;

			maxfd = clientfd;
		}
		int i = 0;
		for (i = sockfd+1; i <= maxfd; i++) {
			if (fds[i].revents & POLLIN) {
				char buffer[128] = { 0 };
				int count = recv(i, buffer, 128, 0);
				if (count == 0) {
					printf("disconnection\n");
					fds[i].fd = -1;
					fds[i].events = 0;
					close(i);
					continue;
				}
				send(i, buffer, 128, 0);
			}
		}
	}

	return 0;
}