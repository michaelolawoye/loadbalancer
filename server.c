#include "LBIncludes.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

enum FDSETC {
	MASTER, READ, WRITE, ERROR
};

fd_set** initFDSets();
int createListenFd();
int handleNewConnection(fd_set** fds, int listenfd);

int handleClientData(int clientfd);
int sendClientMessage(int clientfd);
int handleClientError(int clientfd);

int pollConnections(fd_set** fds, int listenfd, int *maxfd);

int main() {

	fd_set** fds = initFDSets();
	int listenfd = createListenFd();

	int quit = 1;
	int maxfd = 5;

	while (!quit) {
		switch(pollConnections(fds, listenfd, &maxfd)) {

			case READ:
				break;

			case WRITE:
				break;

			case ERROR:
				break;

			default:
				break;
		}

	}

	return 0;
}

fd_set** initFDSets() {

	fd_set* masterFd = (fd_set*)malloc(sizeof(fd_set));
	fd_set* readFd = (fd_set*)malloc(sizeof(fd_set));
	fd_set* writeFd = (fd_set*)malloc(sizeof(fd_set));
	fd_set* errorFd = (fd_set*)malloc(sizeof(fd_set));

	FD_ZERO(masterFd);
	FD_ZERO(readFd);
	FD_ZERO(writeFd);
	FD_ZERO(errorFd);

	fd_set** fds = (fd_set**)malloc(sizeof(fd_set)*4);
	fds[MASTER] = masterFd;
	fds[READ] = readFd;
	fds[WRITE] = writeFd;
	fds[ERROR] = errorFd;

	return fds;

}

int createListenFd() {

	struct addrinfo *myaddr, *curraddr, hints;

	int listenfd;
	int gai_error;
	int sockoptnum;

	char char_port[10];
	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	if ((gai_error = getaddrinfo(nullptr, char_port, &hints, &myaddr)) != 0) {
		printf("createListenFd getaddrinfo() failed. gai_error: %s\n", gai_strerror(gai_error));
		return -1;
	}

	for (curraddr = myaddr; curraddr != NULL; curraddr = curraddr->ai_next) {

		
		if ((listenfd = socket(curraddr->ai_family, curraddr->ai_socktype, curraddr->ai_protocol)) < 0) {
			perror("createListenFd socket() failed, trying next one...");
			continue;
		}

		if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sockoptnum, sizeof(int)) == -1) {
			perror("createListenFd setsockopt()");
			close(listenfd);
			return -1;
		}

		if (bind(listenfd, curraddr->ai_addr, curraddr->ai_addrlen) == -1) {
			perror("createListenFd bind() failed, trying next one...");
			close(listenfd);
			continue;	
		}	
	}

	if (listenfd == -1) {
		printf("createListenFd Couldn't create listen socket.\n");
		return -1;
	}

	freeaddrinfo(myaddr);

	if (listen(listenfd, 10) == -1) {
		perror("createListenFd listen()");
		return -1;
		}

	return listenfd;

}

int handleNewConnection(fd_set** fds, int listenfd) {

	int newfd;
	struct sockaddr_storage their_addr;
	socklen_t their_size = sizeof(struct sockaddr_storage);

	if ((newfd = accept(listenfd, (struct sockaddr*)&their_addr, &their_size)) == -1) {
		perror("handleNewConnection accept()");
		return -1;
	}

	char ipstr[INET6_ADDRSTRLEN];

	inet_ntop(their_addr.ss_family, get_inaddr((struct sockaddr*)&their_addr), ipstr, INET6_ADDRSTRLEN);	

	printf("New connection from %s established\n", ipstr);

	FD_SET(newfd, fds[MASTER]);

	return newfd;
}

int handleClientData(int clientfd) {
	return 1;
}

int sendClientMessage(int clientfd) {
	return 1;
}

int handleClientError(int clientfd) {
	return 1;
}

int pollConnections(fd_set** fds, int listenfd, int* maxfd) {

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;	
	FD_ZERO(fds[READ]);
	FD_ZERO(fds[WRITE]);
	FD_ZERO(fds[ERROR]);


	*(fds[READ]) = *(fds[MASTER]);
	*(fds[WRITE]) = *(fds[MASTER]);
	*(fds[ERROR]) = *(fds[MASTER]);

	FD_CLR(listenfd, fds[WRITE]);
	FD_CLR(listenfd, fds[ERROR]);

	int s = select((*maxfd)+1, fds[READ], fds[WRITE], fds[ERROR], &tv);

	for (int i = 0; i < *maxfd; i++) {

		if (FD_ISSET(i, fds[READ])) {
			printf("Client %d is ready to be read\n", i);

			if (i == listenfd) {
				printf("New connection\n");
				*maxfd = max(handleNewConnection(fds, listenfd), *maxfd);
			} else {
				handleClientData(i);
			}
		}

		if (FD_ISSET(i, fds[WRITE])) {
			printf("Client %d is ready to recieve message\n", i);
			sendClientMessage(i);
		}

		if (FD_ISSET(i, fds[ERROR])) {
			handleClientError(i);
		}
	}
	return 0;
}