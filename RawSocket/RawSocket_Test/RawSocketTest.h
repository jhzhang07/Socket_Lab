#ifndef _RAW_SOCKET_TEST_H_
#define _RAW_SOCKET_TEST_H_

/*
	ע�⣺
	ͷ�ļ�.hֻ���������ͺ�������Ҫ��ͷ�ļ��ﶨ�����������Ҫ��Ȼ#ifndef .. #define ...#endif û����Ŷ^_^
*/

#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include "PacketStruct.h"
#pragma comment(lib, "ws2_32.lib")
// ��������������Ҫ��ͷ�ļ�
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

// ����ԭʼ�׽���, ͬʱ��IP��ַ, ������������Ϊ����ģʽ, ��̽�������ݰ�
BOOL InitRawSocket();

//ѡ��һ��IP��ַ��������̽ 
char* ChooseIP();

// �������ݰ�
BOOL ReceivePacket();

// ���ݰ���ʾ
BOOL ReceivePacket_Print();

// ������
BOOL ExitRawSocket();

// �������ݰ�
void AnalyseRecvPacket(BYTE *lpBuf);

// �򵥷������ݰ�
void AnalyseRecvPacket_All(BYTE *lpBuf);

// ����UDP���ݰ�
void AnalyseRecvPacket_UDP(BYTE *lpBuf);

// ����TCP���ݰ�
void AnalyseRecvPacket_TCP(BYTE *lpBuf);

// �������
void PrintData(BYTE *lpBuf, int iLen, int iPrintType);

// ��ʾ
// ��������������
void MyPrintf(const char * _Format, ...);    

// �����ݴ洢Ϊ�ļ�
void SaveToFile(char *lpszFileName, char *lpBuf);


#endif