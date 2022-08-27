#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <direct.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib, "WS2_32")

#include"message.h"

using namespace std;

//const int BUF_SIZE = 1024;

char port[] = "40000";//控制端口号
//数据端口号由程序生成

void ErrorHandling(string message);
unsigned WINAPI sendFile(void * arg);

bool port_used[100] = { false };//使用40100-40199作为数据连接的端口号，这里查看端口是否被使用过
const int start = 40100;

struct send_file_thread_parameter {//发送文件线程的参数
	char fport[10];
	char file_path[100];
};

int main()
{	
	//控制连接
	WSADATA wsaData;
	SOCKET serverSock, clientSock;
	SOCKADDR_IN serverAddr, clientAddr;
	int size_of_clientAddr;
	fd_set reads, copyReads;
	TIMEVAL timeout;

	char send_message[BUF_SIZE];
	char recv_message[BUF_SIZE];
	int str_len, fdNum, i;

	clientMessage * cmsg;
	serverMessage smsg;

	HANDLE hSendFileThread;

	HANDLE hff;
	WIN32_FIND_DATA fd;
	char filerecords[20][36];//文件上限20个
	int number_of_files = 0;
	char filerecord[701];
	//搜索文件
	hff = FindFirstFile(".\\files\\*", &fd);
	if (hff == INVALID_HANDLE_VALUE)  //发生错误
	{
		ErrorHandling("无法获取文件列表！");
	}
	else {
		i = 0;
		BOOL fMoreFiles = TRUE;
		while (fMoreFiles) {
			if (fd.nFileSizeLow != 0) {//不是目录
				//文件名和文件大小:
				sprintf(filerecords[i], "%20s    %10d\n", fd.cFileName, fd.nFileSizeLow);
				i++;
				if (i == 20)break;
			}
			//搜索下一个文件
			fMoreFiles = FindNextFile(hff, &fd);
		}
		number_of_files = i;
	}
	for (i = 0; i < number_of_files; i++) {
		for (int j = 0; j < 35; j++) {
			filerecord[i * 35 + j] = filerecords[i][j];
		}
	}
	filerecord[(i - 1) * 35 + 36] = '\0';

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}

	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSock == INVALID_SOCKET) {
		ErrorHandling("（控制连接）socket() error!");
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi(port));

	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		ErrorHandling("（控制连接）bind() error!");
	}

	if (listen(serverSock, 5) == SOCKET_ERROR) {
		ErrorHandling("（控制连接）listen() error!");
	}

	FD_ZERO(&reads);
	FD_SET(serverSock, &reads);

	while (1) {
		copyReads = reads;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

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
					str_len = recv(reads.fd_array[i], recv_message, BUF_SIZE, 0);
					if (str_len == -1) {
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(copyReads.fd_array[i]);
						cout << "已断开：" << copyReads.fd_array[i] << endl;
					}
					else {
						recv_message[str_len] = '\0';
						cmsg = new clientMessage(recv_message);
						if (cmsg->getID() == 1) { //ls，发送文件列表
							smsg.changeID(1);
							smsg.changeContent(filerecord);
							smsg.toString(send_message);
							send(reads.fd_array[i], send_message, strlen(send_message), 0);
						}
						else if (cmsg->getID() == 2) {//get，发送文件
							char filename[100];
							struct send_file_thread_parameter * pa = new struct send_file_thread_parameter;
							cmsg->getContent(filename);
							strcpy(pa->file_path, "files\\");
							strcat(pa->file_path, filename);
							FILE * fp;
							if ((fp = fopen(pa->file_path, "rb")) != NULL) {//如果文件存在
								//发送提示消息表示文件存在可以下载
								int _fport;
								for (int j = 0; j < 100; j++) {
									if (!port_used[j]) {
										_fport = start + j;
										port_used[j] = true;
										break;
									}
								}
								_itoa(_fport, pa->fport, 10);
								smsg.changeID(2);
								smsg.changeContent(pa->fport);//数据连接的服务端端口号
								smsg.toString(send_message);
								send(reads.fd_array[i], send_message, strlen(send_message), 0);

								hSendFileThread = (HANDLE)_beginthreadex(NULL, 0, sendFile, (void*)pa, 0, NULL);//把新端口号作为参数传递给线程
							}
							else {//如果文件不存在
								smsg.changeID(3);
								smsg.changeContent("");
								smsg.toString(send_message);
								send(reads.fd_array[i], send_message, strlen(send_message), 0);
							}

						}
						else {
							send(reads.fd_array[i], "无效命令！", 12, 0);
						}
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

unsigned WINAPI sendFile(void * arg) { //传输文件线程
	//FTP：主动方式、被动方式
	//这里采用被动方式
	//数据连接
	struct send_file_thread_parameter * pa = (struct send_file_thread_parameter *)arg;
	char _fport[10];//新申请空间，防止地址共用导致可能发生的错误
	char _filepath[100];
	strcpy_s(_fport, pa->fport);
	strcpy_s(_filepath, pa->file_path);
	SOCKET fserverSock, fclientSock;
	SOCKADDR_IN fserverAddr, fclientAddr;
	int size_of_fclientAddr;
	char fsend_message[BUF_SIZE];
	char frecv_message[BUF_SIZE];
	int str_len;
	//建立新的临时套接字
	fserverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (fserverSock == INVALID_SOCKET) {
		cout << "（数据连接）socket() error!" << endl;
		return -1;
	}
	memset(&fserverAddr, 0, sizeof(fserverAddr));
	fserverAddr.sin_family = AF_INET;
	fserverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	fserverAddr.sin_port = htons(atoi(_fport));
	if (bind(fserverSock, (SOCKADDR*)&fserverAddr, sizeof(fserverAddr)) == SOCKET_ERROR) {
		cout << "（数据连接）bind() error!" << endl;
		closesocket(fserverSock);
		return -1;
	}
	if (listen(fserverSock, 5) == SOCKET_ERROR) {
		cout << "（数据连接）listen() error!" << endl;
		closesocket(fserverSock);
		return -1;
	}
	//等待客户端连接
	size_of_fclientAddr = sizeof(fclientAddr);
	fclientSock = accept(fserverSock, (SOCKADDR*)&fclientAddr, &size_of_fclientAddr);
	//打开文件
	const char *filename = _filepath;
	FILE *fp = fopen(filename, "rb");  //以二进制方式打开文件
	if (fp == NULL) {
		cout << "无法打开文件" << endl;
		closesocket(fserverSock);
		return -1;
	}
	//传输文件
	cout << "端口" << _fport << "发送文件中" << endl;
	while ((str_len = fread(fsend_message, 1, BUF_SIZE, fp)) > 0) {
		send(fclientSock, fsend_message, str_len, 0);
	}
	shutdown(fclientSock, SD_SEND);  //文件读取完毕，断开输出流，向客户端发送FIN包
	recv(fclientSock, frecv_message, BUF_SIZE, 0);  //阻塞，等待客户端接收完毕
	cout << "端口" << _fport << "文件发送完成" << endl;
	//关闭
	fclose(fp);
	closesocket(fclientSock);
	closesocket(fserverSock);
	port_used[ntohs(fserverAddr.sin_port) - start] = false;
	return 0;
}