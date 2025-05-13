#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<error.h>
#include<unistd.d>
#include<pthread.h>


int mian()
{
	int sockfd = socket(IF_INET, SOCK_STREAM, 0);
	struct sockaddr_in saddr;
	saddr.sin_family = IF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(2048);
	if (-1 == bind(sockfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr))) {
		perror("bind");
		return -1;
	}
	listen(sockfd, 10);
#if 0
	while (1) {
		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);
		int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

		pthread thid;
		pthread_create(&thid, NULL, client_thread, &clientfd);
	}
#else //select
	//int select(mxfd, rset, wset, eset, timeout);

	fd_set rfds, rset;
	FD_ZERO(rfds);
	FD_SET(sockfd, &rfds);

	int maxfd = sockfd;

	while (1) {
		rset = rfds;
		int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset)) {
			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
			printf("sockfd:%d\n", clientfd);

			FD_SET(clientfd, &rfds);
			maxfd = clientfd;
		}
		int i = 0;
		for (i = sockfd + 1, i <= maxfd; i++) {
			if (FD_SET(i, &rset)) {
				char buffer[128] = { 0 };
				int count = recv(i, buffer, 128, 0);
				if (count == 0) {
					printf("disconnect\n");
					close(i); 
					break;
				}
				send(i, buffer, 128, 0);
				printf("clientfd:%d, count:%d, buffer:%s\n", i, count,buffer);
			}
		}
	}


#endif
	return 0;
}
