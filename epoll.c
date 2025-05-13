#include<stdio.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
#include<error.h>
#include<sys/epoll.h>
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

	int epfd = epoll_create(1);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	struct epoll_events events[1024] = { 0 };

	while (1) {
		int nready = epoll_wait(epfd, evnts, 1024, -1);
		int i = 0;
		for (i = 0; i < nready; i++) {

			int connfd = events[i].data.fd;
			if (sockfd == connfd) {
				struct sockaddr_in clientaddr;
				socklen_t len = sizeof(clientaddr);
				int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
				printf("clientfd: %d", clientfd);

				ev.events = EPOLLIN;
				ev.data.fd = clientfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
			}
			else if (events[i].events & EPOLLIN) {
				char buffer[128] = { 0 };
				int count = recv(i, buffer, 128, 0);
				if (count == 0) {
					printf("disconnection\n");
					epoll_ctl(epfd, EPOLL_CTL_DEl, connfd, NULL);
					close(i);
					continue;
				}
				send(i, buffer, 128, 0);
			}
		}
	}

#endif
	return 0;
}