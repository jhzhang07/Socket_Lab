#ifndef _RAW_SOCKET_TEST_H_
#define _RAW_SOCKET_TEST_H_

/*
	注意：
	头文件.h只声明变量和函数，不要在头文件里定义变量或函数，要不然#ifndef .. #define ...#endif 没有用哦^_^
*/

#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include "PacketStruct.h"
#pragma comment(lib, "ws2_32.lib")
// 不定参数函数需要的头文件
#include <stdarg.h>     


typedef struct _HOSTIP
{
	int iLen;
	char szIPArray[10][50];
}HOSTIP;

extern SOCKET g_RawSocket;
extern HOSTIP g_HostIp;
extern BOOL g_bStopRecv;

void ShowError(char *lpszText);

// 创建原始套接字, 同时绑定IP地址, 并将网卡设置为混杂模式, 嗅探所有数据包
BOOL InitRawSocket();

//选择一个IP地址来进行嗅探 
char* ChooseIP();

// 接收数据包
BOOL ReceivePacket();

// 数据包显示
BOOL ReceivePacket_Print();

// 清理工作
BOOL ExitRawSocket();

// 分析数据包
void AnalyseRecvPacket(BYTE *lpBuf);

// 简单分析数据包
void AnalyseRecvPacket_All(BYTE *lpBuf);

// 分析UDP数据包
void AnalyseRecvPacket_UDP(BYTE *lpBuf);

// 分析TCP数据包
void AnalyseRecvPacket_TCP(BYTE *lpBuf);

// 输出数据
void PrintData(BYTE *lpBuf, int iLen, int iPrintType);

// 显示
// 不定长参数函数
void MyPrintf(const char * _Format, ...);    

// 将数据存储为文件
void SaveToFile(char *lpszFileName, char *lpBuf);


#endif