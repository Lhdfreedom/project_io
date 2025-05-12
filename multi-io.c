
#include<stdio.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/poll.h>
#include<sys/epoll.h>
//tcp 的 server端

//void *client_thread(void* arg) {
//	int clientfd = *(int *)arg;
//	while (1) {
//		char buffer[128] = { 0 };
//		int count = recv(clientfd, buffer, 128, 0);
//		if (count == 0) {
//			break;
//		}
//		send(clientfd, buffer, count, 0);
//		printf("clientfd: %d, count:%d, buffer:%s\n",clientfd, count, buffer);
//	}
//	close(clientfd);
//}


int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(struct sockaddr_in));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serveraddr.sin_port = htons(2048);

	if(-1 == bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr)))
	{
		perror("bind");
		return -1;
	}

	listen(sockfd, 10);

#if 0

	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);  // accept 是一个阻塞的函数
	printf("accept!\n");
#if 0
	char buffer[128] = { 0 };
	int count = recv(clientfd, buffer, 128, 0);
	send(clientfd, buffer, count, 0);
	printf("sockfd: %d, clientfd: %d, count:%d, buffer:%s\n", sockfd, clientfd, count, buffer);
#else
	while(1){
		char buffer[128] = { 0 };
		int count = recv(clientfd, buffer, 128, 0);
		if (count == 0) {
			break;
		}
		send(clientfd, buffer, count, 0);
		printf("sockfd: %d, clientfd: %d, count:%d, buffer:%s\n", sockfd, clientfd, count, buffer);
	}
#endif

#elif 0

	while (1) {
		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);
		int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);  // accept 是一个阻塞的函数

		pthread_t thid;
		pthread_create(&thid, NULL, client_thread, &clientfd);
	}

#elif 0 //select
	//int nready = select(maxfd, rset, wset, eset, timeout);

	fd_set rfds, rset;

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);

	int maxfd = sockfd; 

	printf("loop\n");

	while (1) {
		rset = rfds;
		int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		
		if (FD_ISSET(sockfd, &rset)) {
			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
			printf("sockfd: %d\n", clientfd);

			FD_SET(clientfd, &rfds);
			maxfd = clientfd;
		}
		int i = 0;
		for (i = sockfd+1; i <= maxfd; i++)
		{
			if (FD_ISSET(i, &rset))
			{
				char buffer[128] = { 0 };
				int count = recv(i, buffer, 128, 0);
				if (count == 0) {
					printf("disconnect\n");
					FD_CLR(i, &rfds);
					close(i);
					break;
				}
				send(i, buffer, count, 0);
				printf("clientfd: %d, count:%d, buffer:%s\n", i, count, buffer);
			}
		}
	}
#elif 0 //poll
	struct pollfd fds[1024] = { 0 };
	fds[sockfd].fd = sockfd;
	fds[sockfd].events = POLLIN;

	int maxfd = sockfd;
	while (1)
	{
		int nready = poll(fds, maxfd+1, -1);
		if (fds[scokfd].revents & POLLIN)
		{
			struct sockaddr_in clientfd;
			socklen_t len = sizeof(clientaddr);

			int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
			
			printf("sockfd: %d", clientfd);
			fds[clientfd].fd = sockfd;
			fds[clientfd].events = POLLIN;
			maxfd = clientfd;
		}

		int i = 0;
		for (i = sockfd+1; i <= maxfd; i++)
		{
			if (fds[i].revents & POLLIN)
			{
				char buffer[128] = { 0 };
				int count = recv(i, buffer, 128, 0);
				if (count == 0) {
					printf("disconnect\n");
					fds[i].fd = -1;
					fds[i].events = 0;
					close(i);
					continue;
				}
				send(i, buffer, count, 0);
				printf("clientfd: %d, count:%d, buffer:%s\n", i, count, buffer);
			}
		}
	}

#else
/*
epoll有水平触发和边缘触发水平触发应该是当文件描述符就绪时，
只要状态没被处理，就会一直通知。而边缘触发则只在状态变化时通知一次，
比如从不可读到可读的时候。这时候需要确保一次性处理完所有数据，否则可能会漏掉事件。
*/
	int epfd = epoll_create(1);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	struct epoll_event events[1024] = { 0 };

	while (1)
	{
		int nready = epoll_wait(epfd, events, 1024, -1);

		int i = 0;
		for (i = 0; i < nready; i++)
		{
			int connfd = events[i].data.fd;
			if (connfd == sockfd)
			{

				struct sockaddr_in clientaddr;
				socklen_t len = sizeof(clientaddr);

				int clientfd = accept(connfd, (struct sockaddr*)&clientaddr, &len);

				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = clientfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

				printf("clientfd: %d\n", clientfd);

			}else if (events[i].events & EPOLLIN) 
			{
				char buffer[10] = { 0 };
				int count = recv(connfd, buffer, 10, 0);
				if (count == 0) {
					printf("disconnect\n");
					epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
					close(count);
					continue;
				}
				send(connfd, buffer, count, 0);
				printf("clientfd: %d, count:%d, buffer:%s\n", connfd, count, buffer);
			}
		}
	}
	//面向io编程                          面向事件	
	/*
	* if(listenfd){						if(EPOLLIN){}else if(events & EPOLLIN){}
	*	accept();
	* }else{
	*	recv();
	*	send();
	  }
	*/

#endif

	//分别为 3  4     0，1，2分别代表stdin stdout stderr  


	return 0;
} 