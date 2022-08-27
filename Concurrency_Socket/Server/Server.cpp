#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <time.h>
#pragma comment(lib, "WS2_32")

using namespace std;

void ErrorHandling(string message);

int main()
{
	const int BUF_SIZE = 1024;

	WSADATA wsaData;
	SOCKET serverSock, clientSock;
	SOCKADDR_IN serverAddr, clientAddr;
	int size_of_clientAddr;
	fd_set reads, copyReads;
	TIMEVAL timeout;

	char buf[BUF_SIZE];// 接收缓冲区  
	int strlen, fdNum, i;

	time_t rawtime;
	struct tm *timeinfo;
	char port[10];

	cout << "port:";
	cin >> port;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}

	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSock == INVALID_SOCKET) {
		ErrorHandling("socket() error!");
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi(port));

	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		ErrorHandling("bind() error!");
	}

	if (listen(serverSock, 5) == SOCKET_ERROR) {
		ErrorHandling("listen() error!");
	}

	FD_ZERO(&reads);// 清空套接字集合
	FD_SET(serverSock, &reads);// 将serverSock套接字加入套接字集合中   

	while (1) {
		copyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;
		// 调用select函数
		if ((fdNum = select(0, &copyReads, 0, 0, &timeout)) == SOCKET_ERROR) {
			break;
		}
		if (fdNum == 0) {
			continue;
		}

		for (i = 0; i < reads.fd_count; i++) {
			if (FD_ISSET(reads.fd_array[i], &copyReads)) {
				if (reads.fd_array[i] == serverSock) {//连接请求
					size_of_clientAddr = sizeof(clientAddr);
					clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &size_of_clientAddr);
					FD_SET(clientSock, &reads);
					cout << "已连接：" << clientSock << endl;
				}
				else {//收到信息
					strlen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (strlen == -1) {
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(copyReads.fd_array[i]);
						cout << "已断开：" << copyReads.fd_array[i] << endl;
					}
					else {
						time(&rawtime);
						timeinfo = localtime(&rawtime);
						char * asc_time = new char[50];
						asc_time = asctime(timeinfo);
						send(reads.fd_array[i], asc_time, 50, 0);
					}
				}
			}
		}
	}

	closesocket(serverSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(string message) {
	cout << message << endl;
	exit(1);
}