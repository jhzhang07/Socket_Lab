#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib, "WS2_32")

#include "message.h"

using namespace std;

//const int BUF_SIZE = 1024;

char port[] = "40000";//控制端口号

void ErrorHandling(string message);

int main()
{
	//控制连接
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN serverAddr;
	char ip[30];
	char send_message[BUF_SIZE];
	char recv_message[BUF_SIZE];
	int str_len;

	clientMessage cmsg;
	string command;
	char filename[100];
	char filepath[100];
	serverMessage * smsg;
	char filenames[701];

	//数据连接
	SOCKET fsock;
	SOCKADDR_IN fserverAddr;
	//char fsend_message[BUF_SIZE];
	char frecv_message[BUF_SIZE];

	cout << "ip:";
	cin >> ip;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		ErrorHandling("（控制连接）socket() error!");
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(ip);
	serverAddr.sin_port = htons(atoi(port));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		ErrorHandling("（控制连接）connect() error!");
	}
	else {
		puts("已连接到服务器\n");
	}

	while (1) {
		cout << "请输入命令：" << endl;
		cin >> command;
		if (command == "ls") {
			cmsg.changeID(1);
			cmsg.changeContent("");
		}
		else if (command == "get") {
			cmsg.changeID(2);
			cout << "请输入要下载文件的文件名：" << endl;
			cin >> filename;
			cmsg.changeContent(filename);
		}
		else {
			cout << "命令无效！" << endl;
			continue;
		}
		cmsg.toString(send_message);
		send(sock, send_message, strlen(send_message), 0);
		str_len = recv(sock, recv_message, BUF_SIZE - 1, 0);
		recv_message[str_len] = 0;
		smsg = new serverMessage(recv_message);
		if (smsg->getID() == 1) { //文件列表
			cout << "文件列表：" << endl;
			cout << "        文件名            大小" << endl;
			smsg->getContent(filenames);
			cout << filenames;
			cout << endl;
		}
		else if (smsg->getID() == 2) { //文件存在
			cout << "文件存在，准备下载......" << endl;
			char fport[10];
			smsg->getContent(fport);
			//数据连接
			fsock = socket(PF_INET, SOCK_STREAM, 0);
			if (fsock == INVALID_SOCKET) {
				closesocket(fsock);
				cout << "（数据连接）socket() error!" << endl;
				continue;
			}
			memset(&fserverAddr, 0, sizeof(fserverAddr));
			fserverAddr.sin_family = AF_INET;
			fserverAddr.sin_addr.s_addr = inet_addr(ip);
			fserverAddr.sin_port = htons(atoi(fport));//从服务端接收端口号
			if (connect(fsock, (SOCKADDR*)&fserverAddr, sizeof(fserverAddr)) == SOCKET_ERROR) {
				closesocket(fsock);
				cout << "（数据连接）connect() error!" << endl;
			}
			//创建空文件
			strcpy(filepath, "files\\");
			strcat(filepath, filename);
			FILE *fp = fopen(filepath, "wb");  //以二进制方式打开文件
			if (fp == NULL) {
				cout << "无法打开文件" << endl;
				closesocket(fsock);
				continue;
			}
			//接收数据
			while ((str_len = recv(fsock, frecv_message, BUF_SIZE, 0)) > 0) {
				fwrite(frecv_message, str_len, 1, fp);
			}
			fclose(fp);
			cout << "文件接收完成" << endl;
			//关闭连接
			closesocket(fsock);
		}
		else if (smsg->getID() == 3) { //文件不存在
			cout << "文件不存在！" << endl;
		}
		else {
			cout << "error" << endl;
		}
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(string message) {
	cout << message << endl;
	exit(1);
}
