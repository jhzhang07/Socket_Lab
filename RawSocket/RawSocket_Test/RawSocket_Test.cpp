// RawSocket_Test.cpp : 定义控制台应用程序的入口点。
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
	printf("    使用说明：\n");
	printf("        1. 请输入数字，请根据IP地址来选择对应的网卡进行嗅探\n");
	printf("        2. 在抓包的过程中，你可以按“回车键”结束嗅探\n");
	printf("*************************************************************\n\n\n");

	InitRawSocket();

	::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvThreadProc, NULL, 0, NULL);

	getchar();

	ExitRawSocket();

	system("pause");

	return 0;
}

