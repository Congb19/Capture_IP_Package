//
// Created by Congb on 2019/1/12.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <mstcpip.h>
#include <string>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;
struct IP_head //IP�ײ�
{
    BYTE version_len;			// IPЭ��汾��IP�ײ�����
    BYTE ser_type;				// ��������
    WORD wPacketLen;			// IP�����ܳ���
    WORD identification;		// ��ʶ  һ�����
    union
    {
        WORD flag;				// ��־
        WORD flagof;			// �ֶ�ƫ��
    };
    BYTE TTL;					// ����ʱ��
    BYTE Protocol_Type;			// Э�����ͣ���PROTOCOL_TYPE����
    WORD Head_checksum;			// IP�ײ�У���
    DWORD Source_ip;			// Դ��ַ
    DWORD Destination_ip;       // Ŀ�ĵ�ַ
    BYTE Options;				// ѡ��
};
int cnt;						// ��¼���
int Get_IPdata(char *buf, int len)
{
    int n = len;
    if (n >= sizeof(IP_head))
    {
        IP_head iphead;
        iphead = *(IP_head*)buf;
        cout << "******************************************************************" << endl ;
        cout << "�� " << cnt++ << " ��IP���ݰ���" << endl;
        cout << "������������������������������������������������������������������" << endl ;
        cout << "��    ��:" << "ipv"<< (iphead.version_len >> 4) << endl;
        cout << "�ײ�����:" << ((iphead.version_len & 0x0F) << 2) << endl;//��λΪ4�ֽ�
        cout << "��������:Priority: " << (iphead.ser_type >> 5) << ",Service: " << ((iphead.ser_type >> 1) & 0x0f) << endl;
        cout << "�� �� ��:" << ntohs(iphead.wPacketLen) << endl; //�����ֽ���תΪ�����ֽ���
        cout << "��    ʶ:" << ntohs(iphead.identification) << endl;
        cout << "��    ־:" << "DF=" << ((iphead.flag >> 14) & 0x01) << ",MF=" << ((iphead.flag >> 13) & 0x01) << endl;
        cout << "Ƭ ƫ ��:" << (iphead.flagof & 0x1fff) << endl;
        cout << "����ʱ��:" << (int)iphead.TTL << endl;
        cout << "Э������:" << int(iphead.Protocol_Type) << endl;
        cout << "У �� ��:" << ntohs(iphead.Head_checksum) << endl;
        cout << "Դ �� ַ:" << inet_ntoa(*(in_addr*)&iphead.Source_ip) << endl;
        cout << "Ŀ�ĵ�ַ:" << inet_ntoa(*(in_addr*)&iphead.Destination_ip) << endl;
        cout << "******************************************************************" << endl << endl;
    }
    return 0;
}
void AutoWSACleanup()
{
    ::WSACleanup();
}
int main()
{
    int n;
    WSADATA wd;	// �����洢��WSAStartup�������ú󷵻ص�Windows Sockets����
    n = WSAStartup(MAKEWORD(2, 2), &wd); // WSAStartup() ������Ӧ��socket���
    if (n)
    {
        cout << "WSAStartup��������" << endl;
        return -1;
    }
    atexit(AutoWSACleanup);	// atexit()�����������������˳�ʱ���õĵǼǺ���
    // WSACleanup()��������ֹWinsock 2 DLL(Ws2_32.dll) ��ʹ��
    // ����SOCKET
    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    if (sock == INVALID_SOCKET)
    {
        cout << WSAGetLastError(); // �ú��������ϴη������������
        return 0;
    }
    // ��ȡ������ַ
    char  name[128];
    if (-1 == gethostname(name, sizeof(name)))	// gethostname()���ر��������ı�׼��������
    {
        closesocket(sock);	// �ر�һ���׽ӿ�
        cout << WSAGetLastError();
        return 0;
    }
    struct hostent * pHostent;
    pHostent = gethostbyname(name);	// gethostbyname()���ض�Ӧ�ڸ����������İ����������ֺ͵�ַ��Ϣ��hostent�ṹ��ָ��
    // �󶨱��ص�ַ��SOCKET
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = *(in_addr*)pHostent->h_addr_list[0]; //IP
    addr.sin_port = 8888; //�˿ڣ�IP��˿ڿ�������
    if (SOCKET_ERROR == bind(sock, (sockaddr *)&addr, sizeof(addr)))
    {
        closesocket(sock);
        cout << WSAGetLastError();
        return 0;
    }
    // ���ø�SOCKETΪ�������������󶨵�IP���������������� �������պͷ��͵����ݰ�
    u_long sioarg = 1;
    DWORD wt = 0;
    if (SOCKET_ERROR == WSAIoctl(sock, SIO_RCVALL, &sioarg, sizeof(sioarg), NULL, 0, &wt, NULL, NULL))// WSAIoctl()����һ���׽ӿڵ�ģʽ
    {
        closesocket(sock);
        cout << WSAGetLastError();
        return 0;
    }
    // ֻ��Ҫ�������� �������Ϊ����IO ʹ����򵥵�IOģ��
    u_long bioarg = 0;
    if (SOCKET_ERROR == ioctlsocket(sock, FIONBIO, &bioarg))// ioctlsocket()�����ǿ����׽ӿڵ�ģʽ
    {
        closesocket(sock);
        cout << WSAGetLastError();
        return 0;
    }
    // ��ʼ��������
    // ��Ϊǰ���Ѿ�����Ϊ����IO��recv�ڽ��յ�����ǰ���᷵��
    cnt = 1;
    char buf[65535];
    int len = 0;
    string temp;
    do
    {
        len = recv(sock, buf, sizeof(buf), 0);// ��recv()������TCP���ӵ���һ�˽�������
        if (len > 0)
        {
            Get_IPdata(buf, len);
        }
        cout << endl << "����س���ȡ��һ�����ݰ�";
        getline(cin, temp);
        cout << endl;
    } while (len > 0);
    closesocket(sock);
    system("pause");
    return 0;
}
