#include "stdafx.h"
#include "RawSocketTest.h"


// ȫ�ֱ���
SOCKET g_RawSocket = 0;
HOSTIP g_HostIp;
BOOL g_bStopRecv = FALSE;


void ShowError(char *lpszText)
{
	char szErr[MAX_PATH] = {0};
	::wsprintf(szErr, "%s Error!\nError Code Is:%d\n", lpszText, ::GetLastError());
	::MessageBox(NULL, szErr, "ERROR", MB_OK | MB_ICONERROR);
}


// ����ԭʼ�׽���, ͬʱ��IP��ַ, ������������Ϊ����ģʽ, ��̽�������ݰ�
BOOL InitRawSocket()
{
	// ��ʼ��Winsock���񻷾�, ���ð汾
	WSADATA wsaData = {0};
	if(0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		ShowError("WSAStartup");
		return FALSE;
	}
	// ����ԭʼ�׽���
	// ������Windows��û�취��Raw SocketץMAC������ݰ���ֻ��ץ��IP�㼰���ϵ����ݰ�������
	g_RawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_IP);       // ע��˴�������!!!
//	g_RawSocket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (INVALID_SOCKET == g_RawSocket)
	{
		WSACleanup();
		ShowError("socket");
		return FALSE;
	}
	// ѡ��һ��IP��ַ��������̽ 
	char *lpszHostIP = ChooseIP();

	// �����ַ�ṹ
	sockaddr_in SockAddr = {0};
	RtlZeroMemory(&SockAddr, sizeof(sockaddr_in));
	SockAddr.sin_addr.S_un.S_addr = inet_addr(lpszHostIP);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(0);
	// ��
	if (SOCKET_ERROR == ::bind(g_RawSocket, (sockaddr *)(&SockAddr), sizeof(sockaddr_in)))
	{
		closesocket(g_RawSocket);
		WSACleanup();
		ShowError("bind");
		return FALSE;
	}
	// ���û���ģʽ���������ܲ������е����ݰ�
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


// ѡ��һ��IP��ַ��������̽ 
char* ChooseIP()
{
	char *lpszHostIP = NULL;

	// ��ȡ������
	char szHostName[MAX_PATH] = { 0 };
	if (SOCKET_ERROR == ::gethostname(szHostName, MAX_PATH))
	{
		closesocket(g_RawSocket);
		WSACleanup();
		ShowError("gethostname");
		return FALSE;
	}
	// ���ݱ�������ȡ����IP��ַ
	hostent *lpHostent = ::gethostbyname(szHostName);
	if (NULL == lpHostent)
	{
		closesocket(g_RawSocket);
		WSACleanup();
		ShowError("gethostbyname");
		return FALSE;
	}
	// IP��ַת��������IP��ַ
	g_HostIp.iLen = 0;
	::lstrcpy(g_HostIp.szIPArray[g_HostIp.iLen], "127.0.0.1");
	g_HostIp.iLen++;
	while (NULL != (lpHostent->h_addr_list[(g_HostIp.iLen - 1)]))
	{
		lpszHostIP = inet_ntoa(*(in_addr *)lpHostent->h_addr_list[(g_HostIp.iLen - 1)]);
		::lstrcpy(g_HostIp.szIPArray[g_HostIp.iLen], lpszHostIP);
		g_HostIp.iLen++;
	}
	// ѡ��IP��ַ��Ӧ����������̽
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


// �������ݰ�
BOOL ReceivePacket()
{
	sockaddr_in RecvAddr = { 0 };
	int iRecvBytes = 0;
	int iRecvAddrLen = sizeof(sockaddr_in);
	DWORD dwBufSize = 12000;
	BYTE *lpRecvBuf = new BYTE[dwBufSize];
	int i = 0;
	g_bStopRecv = TRUE;
	// ����
	while (g_bStopRecv)
	{
		RtlZeroMemory(&RecvAddr, iRecvAddrLen);
		iRecvBytes = recvfrom(g_RawSocket, (char *)lpRecvBuf, dwBufSize, 0, (sockaddr *)(&RecvAddr), &iRecvAddrLen);
		if (0 < iRecvBytes)
		{
			// ���յ����ݰ�
			// �������ݰ�
			AnalyseRecvPacket(lpRecvBuf);
		}
	}

	// �ͷ��ڴ�
	delete[]lpRecvBuf;
	lpRecvBuf = NULL;

	return TRUE;
}


// ���ݰ���ʾ
BOOL ReceivePacket_Print()
{
	sockaddr_in RecvAddr = { 0 };
	int iRecvBytes = 0;
	int iRecvAddrLen = sizeof(sockaddr_in);
	DWORD dwBufSize = 12000;
	BYTE *lpRecvBuf = new BYTE[dwBufSize];
	int i = 0;
	g_bStopRecv = TRUE;
	// ����
	char szTemp[10] = {0};
	FILE *fp = fopen("RecvInfo.txt", "w+");

	while (g_bStopRecv)
	{
		RtlZeroMemory(&RecvAddr, iRecvAddrLen);
		iRecvBytes = recvfrom(g_RawSocket, (char *)lpRecvBuf, dwBufSize, 0, (sockaddr *)(&RecvAddr), &iRecvAddrLen);
		if (0 < iRecvBytes)
		{
			// ���յ���Ϣ
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
	// �ͷ��ڴ�
	delete[]lpRecvBuf;
	lpRecvBuf = NULL;


	return TRUE;
}


// ��ʾ
void MyPrintf(const char * _Format, ...)
{
	char szTemp[MAX_PATH] = {0};
	// ��1�����������ָ������б�ı���
	va_list arg_ptr;		
	// ��2�������������������ʼ��������ָ������б�
	va_start(arg_ptr, _Format);		
	// ��3������ȡarg_ptrָ��ĵ�ǰ����
	vsprintf(szTemp, _Format, arg_ptr);
	// ��4����������
	va_end(arg_ptr);

	// ��ʾ
	printf("%s", szTemp);
	// ���浽�ļ�
	SaveToFile("RecvInfo.txt", szTemp);
}


// �����ݴ洢Ϊ�ļ�
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


// ������
BOOL ExitRawSocket()
{
	g_bStopRecv = FALSE;
	Sleep(500);
	closesocket(g_RawSocket);
	WSACleanup();

	return TRUE;
}


// �������ݰ�
void AnalyseRecvPacket(BYTE *lpBuf)
{
/*
	������Windows��û�취��Raw SocketץMAC������ݰ���ֻ��ץ��IP�㼰���ϵ����ݰ�������
	ע�⣺
	���ݰ����ֽ�˳��ת�����⣡����
	//����Ҫ�������ֽ���ת��Ϊ�����ֽ���
*/
	//����IPЭ��
	PIPV4HEADER ip = (PIPV4HEADER)lpBuf;
	//����IP����Э������
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
		//����tcpЭ��
		MyPrintf("[TCP]\n");
		AnalyseRecvPacket_TCP(lpBuf);
		break;
	}
	case IPPROTO_UDP:
	{
		//����udpЭ��
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


// �򵥷������ݰ�
void AnalyseRecvPacket_All(BYTE *lpBuf)
{
	struct sockaddr_in saddr, daddr;
	PIPV4HEADER ip = (PIPV4HEADER)lpBuf;
	saddr.sin_addr.s_addr = ip->ipv4_sourpa;
	daddr.sin_addr.s_addr = ip->ipv4_destpa;

	MyPrintf("From:%s --> ", inet_ntoa(saddr.sin_addr));
	MyPrintf("To:%s\n", inet_ntoa(daddr.sin_addr));
}


// ����UDP���ݰ�
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


// ����TCP���ݰ�
void AnalyseRecvPacket_TCP(BYTE *lpBuf)
{
	struct sockaddr_in saddr, daddr;
	PIPV4HEADER ip = (PIPV4HEADER)lpBuf;
	PTCPHEADER tcp = (PTCPHEADER)(lpBuf + (ip->ipv4_ver_hl & 0x0F) * 4);
	int hlen = (ip->ipv4_ver_hl & 0x0F) * 4 + tcp->tcp_hlen * 4;
	int dlen = ntohs(ip->ipv4_plen) - hlen;    //����Ҫ�������ֽ���ת��Ϊ�����ֽ���
	saddr.sin_addr.s_addr = ip->ipv4_sourpa;
	daddr.sin_addr.s_addr = ip->ipv4_destpa;
	
	MyPrintf("Protocol:TCP  ");
	MyPrintf("From:%s:%d --> ", inet_ntoa(saddr.sin_addr), ntohs(tcp->tcp_sourport));
	MyPrintf("To:%s:%d  ", inet_ntoa(daddr.sin_addr), ntohs(tcp->tcp_destport));
	MyPrintf("ack:%u  syn:%u length=%d\n", tcp->tcp_acknu, tcp->tcp_seqnu, dlen);

	PrintData((lpBuf + hlen), dlen, 0);
}


// �������
void PrintData(BYTE *lpBuf, int iLen, int iPrintType)
{
	if (0 == iPrintType)   // 16����
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
	else if (1 == iPrintType) // ASCII����
	{
		for (int i = 0; i < iLen; i++)
		{
			MyPrintf("%c", lpBuf[i]);
		}
		MyPrintf("\n");
	}
}