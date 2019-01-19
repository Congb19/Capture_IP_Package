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
struct IP_head //IP首部
{
    BYTE version_len;			// IP协议版本和IP首部长度
    BYTE ser_type;				// 服务类型
    WORD wPacketLen;			// IP包的总长度
    WORD identification;		// 标识  一般递增
    union
    {
        WORD flag;				// 标志
        WORD flagof;			// 分段偏移
    };
    BYTE TTL;					// 生存时间
    BYTE Protocol_Type;			// 协议类型，见PROTOCOL_TYPE定义
    WORD Head_checksum;			// IP首部校验和
    DWORD Source_ip;			// 源地址
    DWORD Destination_ip;       // 目的地址
    BYTE Options;				// 选项
};
int cnt;						// 记录序号
int Get_IPdata(char *buf, int len)
{
    int n = len;
    if (n >= sizeof(IP_head))
    {
        IP_head iphead;
        iphead = *(IP_head*)buf;
        cout << "******************************************************************" << endl ;
        cout << "第 " << cnt++ << " 个IP数据包：" << endl;
        cout << "—————————————————————————————————" << endl ;
        cout << "版    本:" << "ipv"<< (iphead.version_len >> 4) << endl;
        cout << "首部长度:" << ((iphead.version_len & 0x0F) << 2) << endl;//单位为4字节
        cout << "服务类型:Priority: " << (iphead.ser_type >> 5) << ",Service: " << ((iphead.ser_type >> 1) & 0x0f) << endl;
        cout << "总 长 度:" << ntohs(iphead.wPacketLen) << endl; //网络字节序转为主机字节序
        cout << "标    识:" << ntohs(iphead.identification) << endl;
        cout << "标    志:" << "DF=" << ((iphead.flag >> 14) & 0x01) << ",MF=" << ((iphead.flag >> 13) & 0x01) << endl;
        cout << "片 偏 移:" << (iphead.flagof & 0x1fff) << endl;
        cout << "生存时间:" << (int)iphead.TTL << endl;
        cout << "协议类型:" << int(iphead.Protocol_Type) << endl;
        cout << "校 验 和:" << ntohs(iphead.Head_checksum) << endl;
        cout << "源 地 址:" << inet_ntoa(*(in_addr*)&iphead.Source_ip) << endl;
        cout << "目的地址:" << inet_ntoa(*(in_addr*)&iphead.Destination_ip) << endl;
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
    WSADATA wd;	// 用来存储被WSAStartup函数调用后返回的Windows Sockets数据
    n = WSAStartup(MAKEWORD(2, 2), &wd); // WSAStartup() 进行相应的socket库绑定
    if (n)
    {
        cout << "WSAStartup函数错误！" << endl;
        return -1;
    }
    atexit(AutoWSACleanup);	// atexit()函数是在正常程序退出时调用的登记函数
    // WSACleanup()功能是终止Winsock 2 DLL(Ws2_32.dll) 的使用
    // 创建SOCKET
    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    if (sock == INVALID_SOCKET)
    {
        cout << WSAGetLastError(); // 该函数返回上次发生的网络错误
        return 0;
    }
    // 获取本机地址
    char  name[128];
    if (-1 == gethostname(name, sizeof(name)))	// gethostname()返回本地主机的标准主机名。
    {
        closesocket(sock);	// 关闭一个套接口
        cout << WSAGetLastError();
        return 0;
    }
    struct hostent * pHostent;
    pHostent = gethostbyname(name);	// gethostbyname()返回对应于给定主机名的包含主机名字和地址信息的hostent结构的指针
    // 绑定本地地址到SOCKET
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = *(in_addr*)pHostent->h_addr_list[0]; //IP
    addr.sin_port = 8888; //端口，IP层端口可随意填
    if (SOCKET_ERROR == bind(sock, (sockaddr *)&addr, sizeof(addr)))
    {
        closesocket(sock);
        cout << WSAGetLastError();
        return 0;
    }
    // 设置该SOCKET为接收所有流经绑定的IP的网卡的所有数据 包括接收和发送的数据包
    u_long sioarg = 1;
    DWORD wt = 0;
    if (SOCKET_ERROR == WSAIoctl(sock, SIO_RCVALL, &sioarg, sizeof(sioarg), NULL, 0, &wt, NULL, NULL))// WSAIoctl()控制一个套接口的模式
    {
        closesocket(sock);
        cout << WSAGetLastError();
        return 0;
    }
    // 只需要接收数据 因此设置为阻塞IO 使用最简单的IO模型
    u_long bioarg = 0;
    if (SOCKET_ERROR == ioctlsocket(sock, FIONBIO, &bioarg))// ioctlsocket()功能是控制套接口的模式
    {
        closesocket(sock);
        cout << WSAGetLastError();
        return 0;
    }
    // 开始接收数据
    // 因为前面已经设置为阻塞IO，recv在接收到数据前不会返回
    cnt = 1;
    char buf[65535];
    int len = 0;
    string temp;
    do
    {
        len = recv(sock, buf, sizeof(buf), 0);// 用recv()函数从TCP连接的另一端接收数据
        if (len > 0)
        {
            Get_IPdata(buf, len);
        }
        cout << endl << "输入回车获取下一个数据包";
        getline(cin, temp);
        cout << endl;
    } while (len > 0);
    closesocket(sock);
    system("pause");
    return 0;
}
