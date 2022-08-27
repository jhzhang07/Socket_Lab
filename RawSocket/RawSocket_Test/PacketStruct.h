#ifndef _PACKETSTRUCT_H
#define _PACKETSTRUCT_H


#pragma pack(1)


/*��̫��֡ͷ��ʽ�ṹ�� 14���ֽ�*/
typedef struct ether_header
{
	unsigned char ether_dhost[6];// Ŀ��MAC��ַ
	unsigned char ether_shost[6];// ԴMAC��ַ
	unsigned short ether_type;// eh_type��ֵ��Ҫ������һ���Э�飬���Ϊip��Ϊ0��0800
}ETHERHEADER, *PETHERHEADER;

/*��ARP�ֶνṹ�� 28���ֽ�*/
typedef struct arp_header
{
	unsigned short arp_hrd;
	unsigned short arp_pro;
	unsigned char arp_hln;
	unsigned char arp_pln;
	unsigned short arp_op;
	unsigned char arp_sourha[6];
	unsigned long arp_sourpa;
	unsigned char arp_destha[6];
	unsigned long arp_destpa;
}ARPHEADER, *PARPHEADER;

/*ARP���Ľṹ�� 42���ֽ�*/
typedef struct arp_packet
{
	ETHERHEADER etherHeader;
	ARPHEADER   arpHeader;
}ARPPACKET, *PARPPACKET;


// ipv4_pro�ֶΣ�
#define PROTOCOL_ICMP   0x01
#define PROTOCOL_IGMP   0x02
#define PROTOCOL_TCP    0x06
#define PROTOCOL_UDP    0x11

/*IPv4��ͷ�ṹ�� 20���ֽ�*/
typedef struct ipv4_header
{
	unsigned char ipv4_ver_hl;// Version(4 bits) + Internet Header Length(4 bits)���Ȱ�4�ֽڶ���
	unsigned char ipv4_stype; // ��������
	unsigned short ipv4_plen;// �ܳ��ȣ�����IP����ͷ��TCP����ͷ�Լ����ݣ�
	unsigned short ipv4_pidentify;// ID���嵥��IP
	unsigned short ipv4_flag_offset;// ��־λƫ����
	unsigned char ipv4_ttl; // ����ʱ��
	unsigned char ipv4_pro;// Э������
	unsigned short ipv4_crc;// У���
	unsigned long  ipv4_sourpa;// ԴIP��ַ
	unsigned long  ipv4_destpa;// Ŀ��IP��ַ
}IPV4HEADER, *PIPV4HEADER;


/*IPv6��ͷ�ṹ�� 40���ֽ�*/
typedef struct ipv6_header
{
	unsigned char ipv6_ver_hl;
	unsigned char ipv6_priority;
	unsigned short ipv6_lable;
	unsigned short ipv6_plen;
	unsigned char  ipv6_nextheader;
	unsigned char  ipv6_limits;
	unsigned char ipv6_sourpa[16];
	unsigned char ipv6_destpa[16];
}IPV6HEADER, *PIPV6HEADER;

/*TCP��ͷ�ṹ�� 20���ֽ�*/
typedef struct tcp_header
{
	unsigned short tcp_sourport;//Դ�˿�
	unsigned short tcp_destport;//Ŀ�Ķ˿�
	unsigned long  tcp_seqnu;//���к�
	unsigned long  tcp_acknu;//ȷ�Ϻ�
	unsigned char  tcp_hlen; //4λ�ײ�����
	unsigned char  tcp_reserved;//��־λ
	unsigned short tcp_window;//���ڴ�С
	unsigned short tcp_chksum;//�����
	unsigned short tcp_urgpoint;//����ָ��
}TCPHEADER, *PTCPHEADER;

/*UDP��ͷ�ṹ�� 8���ֽ�*/
typedef struct udp_header
{
	unsigned short udp_sourport;// Դ�˿� 
	unsigned short udp_destport;// Ŀ�Ķ˿�
	unsigned short udp_hlen;// ����
	unsigned short udp_crc;// У���
}UDPHEADER, *PUDPHEADER;


#pragma pack()


#endif