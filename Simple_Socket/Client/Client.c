#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "WS2_32")

void ErrorHandling(char *message);

int main()
{
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN serverAddr;
	char ip[30];
	char port[10];
	char send_message[500];
	char recv_message[500];
	int strlen;

	puts("ip:");
	fgets(ip, 30, stdin);
	puts("port:");
	fgets(port, 10, stdin);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		ErrorHandling("socket() error!");
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(ip);
	serverAddr.sin_port = htons(atoi(port));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		ErrorHandling("connect() error!");
	}
	else{
		puts("Connected...\n");
	}

	while (1) {
		fgets(send_message, 500, stdin);
		send(sock, send_message, 500, 0);
		strlen = recv(sock, recv_message, 500 - 1, 0);
		recv_message[strlen] = 0;
		printf("from server: %s", recv_message);
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}