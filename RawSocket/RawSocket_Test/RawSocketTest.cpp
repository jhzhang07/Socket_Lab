#include "stdafx.h"
#include "RawSocketTest.h"


// 全局变量
SOCKET g_RawSocket = 0;
HOSTIP g_HostIp;
BOOL g_bStopRecv = FALSE;


void ShowError(char *lpszText)
{
	char szErr[MAX_PATH] = {0};
	::wsprintf(szErr, "%s Error!\nError Code Is:%d\n", lpszText, ::GetLastError());
	::MessageBox(NULL, szErr, "ERROR", MB_OK | MB_ICONERROR);
}


// 创建原始套接字, 同时绑定IP地址, 并将网卡设置为混杂模式, 嗅探所有数据包
BOOL InitRawSocket()
{
	// 初始化Winsock服务环境, 设置版本
	WSADATA wsaData = {0};
	if(0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ShowError("WSAStartup");
		return FALSE;
	}
	// 创建原始套接字
	// ！！！Windows上没办法用Raw Socket抓MAC层的数据包，只能抓到IP层及以上的数据包！！！
	g_RawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);       // 注意此处的设置!!!
//	g_RawSocket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (INVALID_SOCKET == g_RawSocket)
	{
		WSACleanup();
		ShowError("socket");
		return FALSE;
	}
	// 选择一个IP地址来进行嗅探 
	char *lpszHostIP = ChooseIP();

	// 构造地址结构
	sockaddr_in SockAddr = {0};
	RtlZeroMemory(&SockAddr, sizeof(sockaddr_in));
	SockAddr.sin_addr.S_un.S_addr = inet_addr(lpszHostIP);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(0);
	// 绑定
	if (SOCKET_ERROR == ::bind(g_RawSocket, (sockaddr *)(&SockAddr), sizeof(sockaddr_in)))
	{
		closesocket(g_RawSocket);
		WSACleanup();
		ShowError("bind");
		return FALSE;
	}
	// 设置混杂模式，这样才能捕获所有的数据包
	DWORD dwSetVal = 1;
	if (SOCKET_ERROR == ioctlsocket(g_RawSocket, SIO_RCVALL, &dwSetVal))
	{
		closesocket(g_RawSocket);
		WSACleanup();
		ShowError("ioctlsocket");
		return FALSE;
	}

	return TRUE;
}


// 选择一个IP地址来进行嗅探 
char* ChooseIP()
{
	char *lpszHostIP = NULL;

	// 获取本机名
	char szHostName[MAX_PATH] = { 0 };
	if (SOCKET_ERROR == ::gethostname(szHostName, MAX_PATH))
	{
		closesocket(g_RawSocket);
		WSACleanup();
		ShowError("gethostname");
		return FALSE;
	}
	// 根据本机名获取本机IP地址
	hostent *lpHostent = ::gethostbyname(szHostName);
	if (NULL == lpHostent)
	{
		closesocket(g_RawSocket);
		WSACleanup();
		ShowError("gethostbyname");
		return FALSE;
	}
	// IP地址转换并保存IP地址
	g_HostIp.iLen = 0;
	::lstrcpy(g_HostIp.szIPArray[g_HostIp.iLen], "127.0.0.1");
	g_HostIp.iLen++;
	while (NULL != (lpHostent->h_addr_list[(g_HostIp.iLen - 1)]))
	{
		lpszHostIP = inet_ntoa(*(in_addr *)lpHostent->h_addr_list[(g_HostIp.iLen - 1)]);
		::lstrcpy(g_HostIp.szIPArray[g_HostIp.iLen], lpszHostIP);
		g_HostIp.iLen++;
	}
	// 选择IP地址对应的网卡来嗅探
	printf("Choose A IP Address To Sniff:\n");
	for (int i = 0; i < g_HostIp.iLen; i++)
	{
		printf("\tIP %d:%s\n", i, g_HostIp.szIPArray[i]);
	}
	printf("Input A Number: ");
	int iChoose = 0;
	scanf("%d", &iChoose);
	getchar();
	if ((0 > iChoose) || (iChoose >= g_HostIp.iLen))
	{
		printf("Choose Error!\nExit Now!!!\n");
		system("pause");
		exit(0);
	}
	printf("Sniffing...\n");

	if ((0 <= iChoose) && (iChoose < g_HostIp.iLen))
	{
		lpszHostIP = g_HostIp.szIPArray[iChoose];
	}

	return lpszHostIP;
}


// 接收数据包
BOOL ReceivePacket()
{
	sockaddr_in RecvAddr = { 0 };
	int iRecvBytes = 0;
	int iRecvAddrLen = sizeof(sockaddr_in);
	DWORD dwBufSize = 12000;
	BYTE *lpRecvBuf = new BYTE[dwBufSize];
	int i = 0;
	g_bStopRecv = TRUE;
	// 接收
	while (g_bStopRecv)
	{
		RtlZeroMemory(&RecvAddr, iRecvAddrLen);
		iRecvBytes = recvfrom(g_RawSocket, (char *)lpRecvBuf, dwBufSize, 0, (sockaddr *)(&RecvAddr), &iRecvAddrLen);
		if (0 < iRecvBytes)
		{
			// 接收到数据包
			// 分析数据包
			AnalyseRecvPacket(lpRecvBuf);
		}
	}

	// 释放内存
	delete[]lpRecvBuf;
	lpRecvBuf = NULL;

	return TRUE;
}


// 数据包显示
BOOL ReceivePacket_Print()
{
	sockaddr_in RecvAddr = { 0 };
	int iRecvBytes = 0;
	int iRecvAddrLen = sizeof(sockaddr_in);
	DWORD dwBufSize = 12000;
	BYTE *lpRecvBuf = new BYTE[dwBufSize];
	int i = 0;
	g_bStopRecv = TRUE;
	// 接收
	char szTemp[10] = {0};
	FILE *fp = fopen("RecvInfo.txt", "w+");

	while (g_bStopRecv)
	{
		RtlZeroMemory(&RecvAddr, iRecvAddrLen);
		iRecvBytes = recvfrom(g_RawSocket, (char *)lpRecvBuf, dwBufSize, 0, (sockaddr *)(&RecvAddr), &iRecvAddrLen);
		if (0 < iRecvBytes)
		{
			// 接收到信息
			printf("[RECV] %dbytes\n", iRecvBytes);

			::wsprintf(szTemp, "[RECV] %dbytes\n", iRecvBytes);
			fputs(szTemp, fp);
			for (i = 0; i < iRecvBytes; i++)
			{
				if (!g_bStopRecv)
				{
					break;
				}
				if ((0 == (i % 8)) && (0 != i))
				{
					printf("  ");

					::wsprintf(szTemp, "%s", "   ");
					fputs(szTemp, fp);
				}
				if ((0 == (i % 16)) && (0 != i))
				{
					printf("\n");

					::wsprintf(szTemp, "%s", "\n");
					fputs(szTemp, fp);
				}
				printf("%02x ", lpRecvBuf[i]);
				
				::wsprintf(szTemp, "%02X ", lpRecvBuf[i]);
				fputs(szTemp, fp);

			}
			printf("\n");

			::wsprintf(szTemp, "%s", "\n");
			fputs(szTemp, fp);
		}
	}

	fclose(fp);
	// 释放内存
	delete[]lpRecvBuf;
	lpRecvBuf = NULL;


	return TRUE;
}


// 显示
void MyPrintf(const char * _Format, ...)
{
	char szTemp[MAX_PATH] = {0};
	// 第1步，定义这个指向参数列表的变量
	va_list arg_ptr;		
	// 第2步，把上面这个变量初始化，让它指向参数列表
	va_start(arg_ptr, _Format);		
	// 第3步，获取arg_ptr指向的当前参数
	vsprintf(szTemp, _Format, arg_ptr);
	// 第4步，清理工作
	va_end(arg_ptr);

	// 显示
	printf("%s", szTemp);
	// 保存到文件
	SaveToFile("RecvInfo.txt", szTemp);
}


// 将数据存储为文件
void SaveToFile(char *lpszFileName, char *lpBuf)
{
	FILE *fp = fopen(lpszFileName, "a+");
	if(NULL == fp)
	{
		return ;
	}

	fputs(lpBuf, fp);

	fclose(fp);
}


// 清理工作
BOOL ExitRawSocket()
{
	g_bStopRecv = FALSE;
	Sleep(500);
	closesocket(g_RawSocket);
	WSACleanup();

	return TRUE;
}


// 分析数据包
void AnalyseRecvPacket(BYTE *lpBuf)
{
/*
	！！！Windows上没办法用Raw Socket抓MAC层的数据包，只能抓到IP层及以上的数据包！！！
	注意：
	数据包的字节顺序转换问题！！！
	//这里要将网络字节序转换为本地字节序
*/
	//分析IP协议
	PIPV4HEADER ip = (PIPV4HEADER)lpBuf;
	//分析IP包的协议类型
	switch (ip->ipv4_pro)
	{
	case IPPROTO_ICMP:
	{
		MyPrintf("[ICMP]\n");
		AnalyseRecvPacket_All(lpBuf);
		break;
	}
	case IPPROTO_IGMP:
	{
		MyPrintf("[IGMP]\n");
		AnalyseRecvPacket_All(lpBuf);
		break;
	}
	case IPPROTO_TCP:
	{
		//分析tcp协议
		MyPrintf("[TCP]\n");
		AnalyseRecvPacket_TCP(lpBuf);
		break;
	}
	case IPPROTO_UDP:
	{
		//分析udp协议
		MyPrintf("[UDP]\n");
		AnalyseRecvPacket_UDP(lpBuf);
		break;
	}
	default:
	{
		MyPrintf("[OTHER IP]\n");
		AnalyseRecvPacket_All(lpBuf);
		break;
	}
	}
}


// 简单分析数据包
void AnalyseRecvPacket_All(BYTE *lpBuf)
{
	struct sockaddr_in saddr, daddr;
	PIPV4HEADER ip = (PIPV4HEADER)lpBuf;
	saddr.sin_addr.s_addr = ip->ipv4_sourpa;
	daddr.sin_addr.s_addr = ip->ipv4_destpa;

	MyPrintf("From:%s --> ", inet_ntoa(saddr.sin_addr));
	MyPrintf("To:%s\n", inet_ntoa(daddr.sin_addr));
}


// 分析UDP数据包
void AnalyseRecvPacket_UDP(BYTE *lpBuf)
{
	struct sockaddr_in saddr, daddr;
	PIPV4HEADER ip = (PIPV4HEADER)lpBuf;
	PUDPHEADER udp = (PUDPHEADER)(lpBuf + (ip->ipv4_ver_hl & 0x0F) * 4);
	int hlen = (int)((ip->ipv4_ver_hl & 0x0F) * 4 + sizeof(UDPHEADER));
	int dlen = (int)(ntohs(udp->udp_hlen) - 8);
//	int dlen = (int)(udp->udp_hlen - 8);
	saddr.sin_addr.s_addr = ip->ipv4_sourpa;
	daddr.sin_addr.s_addr = ip->ipv4_destpa;
	MyPrintf("Protocol:UDP  ");
	MyPrintf("From:%s:%d -->", inet_ntoa(saddr.sin_addr), ntohs(udp->udp_sourport));
	MyPrintf("To:%s:%d\n", inet_ntoa(daddr.sin_addr), ntohs(udp->udp_destport));
	
	PrintData((lpBuf + hlen), dlen, 0);
}


// 分析TCP数据包
void AnalyseRecvPacket_TCP(BYTE *lpBuf)
{
	struct sockaddr_in saddr, daddr;
	PIPV4HEADER ip = (PIPV4HEADER)lpBuf;
	PTCPHEADER tcp = (PTCPHEADER)(lpBuf + (ip->ipv4_ver_hl & 0x0F) * 4);
	int hlen = (ip->ipv4_ver_hl & 0x0F) * 4 + tcp->tcp_hlen * 4;
	int dlen = ntohs(ip->ipv4_plen) - hlen;    //这里要将网络字节序转换为本地字节序
	saddr.sin_addr.s_addr = ip->ipv4_sourpa;
	daddr.sin_addr.s_addr = ip->ipv4_destpa;
	
	MyPrintf("Protocol:TCP  ");
	MyPrintf("From:%s:%d --> ", inet_ntoa(saddr.sin_addr), ntohs(tcp->tcp_sourport));
	MyPrintf("To:%s:%d  ", inet_ntoa(daddr.sin_addr), ntohs(tcp->tcp_destport));
	MyPrintf("ack:%u  syn:%u length=%d\n", tcp->tcp_acknu, tcp->tcp_seqnu, dlen);

	PrintData((lpBuf + hlen), dlen, 0);
}


// 输出数据
void PrintData(BYTE *lpBuf, int iLen, int iPrintType)
{
	if (0 == iPrintType)   // 16进制
	{
		for (int i = 0; i < iLen; i++)
		{
			if ((0 == (i % 8)) && (0 != i))
			{
				MyPrintf("  ");
			}
			if ((0 == (i % 16)) && (0 != i))
			{
				MyPrintf("\n");
			}
			MyPrintf("%02x ", lpBuf[i]);

		}
		MyPrintf("\n");
	}
	else if (1 == iPrintType) // ASCII编码
	{
		for (int i = 0; i < iLen; i++)
		{
			MyPrintf("%c", lpBuf[i]);
		}
		MyPrintf("\n");
	}
}