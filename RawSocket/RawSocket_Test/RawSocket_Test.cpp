// RawSocket_Test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "RawSocketTest.h"


UINT RecvThreadProc(LPVOID lpVoid)
{
	ReceivePacket();

	return 0;
}


int main()
{
	printf("***************** Welcome To World Of Demon *****************\n");
	printf("    ʹ��˵����\n");
	printf("        1. ���������֣������IP��ַ��ѡ���Ӧ������������̽\n");
	printf("        2. ��ץ���Ĺ����У�����԰����س�����������̽\n");
	printf("*************************************************************\n\n\n");

	InitRawSocket();

	::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvThreadProc, NULL, 0, NULL);

	getchar();

	ExitRawSocket();

	system("pause");

	return 0;
}

