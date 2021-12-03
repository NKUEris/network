#include<iostream>
#include"Winsock2.h"
#include<fstream>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)
using namespace std;
SOCKET serverSock = INVALID_SOCKET;
SOCKADDR_IN clientAddr;
SOCKADDR_IN serverAddr;
int addrlen = sizeof(SOCKADDR);
int seq = 0;
char temp[10000][1024];
class message
{
public:
	unsigned short flag=0x01;														//标志位 0X01 SYN,0X02 ACK, 0X04 FIN, 0X08 文件开始传输，0XF 文件传输结束
	DWORD DstIP=inet_addr("127.0.0.1"), SrcIP =inet_addr("127.0.0.1");	//源ip和目标ip
	u_short DstPort=3484, SrcPort=3483;											//源端口号和目标端口号
	int msgseq=0;																//消息序号
	int ackseq=0;																//接收ack的消息序号
	int length=0;																//定长报文长度
	int index=0;																//描述消息条数
	unsigned short check=0;																//校验和															
	char msg[1024] = {'0'};															//用于存储消息内容
	message()
	{
		this->flag = 0;
	}
	message(DWORD DstIP, DWORD SrcIP, short DstPort, short SrcPort);
	void set_syn();
	int get_syn();
	int get_ack();
	void set_ack(message Msg);
	void setcheck();
	int checksum();
	void set_fin();
	int get_fin();
	bool get_start()
	{
		return this->flag & 0x08;
	}
	void set_sart()
	{
		if (!get_start()) flag += 0x08;
		return;
	}
	bool get_endfile()
	{
		return this->flag & 0xf;
	}
	void set_end()
	{
		if (!get_endfile()) flag += 0xf;
		return;
	}
	int startfile()
	{
		return this->flag & 0x08;
	}
	int endfile()
	{
		return this->flag & 0xf;
	}
	
};
message::message(DWORD DstIP, DWORD SrcIP, short DstPort, short SrcPort)
{
	this->DstIP = DstIP;
	this->SrcIP = SrcIP;
	this->DstPort = DstPort;
	this->SrcPort = SrcPort;
	this->msgseq = -1;
	this->index = -1;
	this->check = -1;
	memset(this->msg, 0, sizeof(this->msg));
}
void message::set_syn()
{
	if (get_syn() == 0)
		this->flag += 0x01;
}
int message::get_syn()
{
	//syn= 001
	if (this->flag & 0x01)
		return 1;
	else return 0;
}
int message::get_ack()
{
	//ACK = 010第二位标志位
	if (this->flag & 0x02)
		return 1;
	else return 0;
}
void message::set_ack(message Msg)		//对消息的确认
{
	if (get_ack() == 0)
		this->flag += 0x02;
	this->ackseq = Msg.msgseq;
}
int message::checksum()				//对校验和进行计算
{
	unsigned long checksum = 0;
	unsigned short* buf = (unsigned short*)this;
	for (int i = 0; i < 528; i++)
	{
		checksum += buf[i];
		int t = checksum >> 16;
		checksum = t + (checksum & 0xffff);
	}
	if (checksum & 0xffff == (0xffff))
		return 1;
	return 0;
	//unsigned  sum = 0;
	//u_char* temp = (u_char*)this;
	//for (int i = 0; i < 528; i++)
	//{
	//	sum += temp[2 * i] << 8 + temp[2 * i + 1];
	//	while (sum >=0x10000)					//溢出
	//	{
	//		unsigned t = sum >> 16;//将最高位回滚添加至最低位
	//		sum += t;
	//	}
	//}
	//if (check + sum == 0xffff)return 1;
	//else return 0;
}
void message::setcheck()		//发送方校验和
{
	unsigned long checksum = 0;
	unsigned short* buf = (unsigned short*)this;
	for (int i = 0; i < 528; i++)
	{
		checksum += buf[i];
		int t = checksum >> 16;
		checksum = t + (checksum & 0xffff);
	}
	this->check = ~checksum & 0xffff;
	//unsigned  sum = 0;
	//u_char* temp = (u_char*)this;
	//for (int i = 0; i < 528; i++)
	//{
	//	sum += temp[2 * i] << 8 + temp[2 * i + 1];
	//	while (sum >= 0x10000)					//溢出
	//	{
	//		unsigned short t = sum >> 16;//将最高位回滚添加至最低位
	//		sum += t;
	//	}
	//}
	//this->check = (unsigned short)(~ sum);			//按位取反

}
void message::set_fin()
{
	if (get_fin() == 0)
		this->flag += 0x04;
}
int message::get_fin()
{
	if (this->flag & 0x04)
		return 1;
	else return 0;
}

void Recv(message& Msg)
{
	memset(Msg.msg, 0, sizeof(Msg.msg));
	recvfrom(serverSock, (char*)&Msg, sizeof(message), 0, (SOCKADDR*)&clientAddr, (int*)&addrlen);
}
void Send(message& Msg)
{
	Msg.setcheck();
	sendto(serverSock, (char*)&Msg, sizeof(message), 0, (SOCKADDR*)&clientAddr, sizeof(SOCKADDR));
}

int buildConnection(message Msg)
{
	message send = message();
	send.set_ack(Msg);
	send.msgseq = seq++;
	Send(send);
	cout << "连接成功" << endl;
	return 1;
}
int disconnect(message Msg)
{
	message send = message();
	send.set_ack(Msg);
	send.msgseq = seq++;
	Send(send);
	cout << "客户端已经断开链接....." << endl;
	return 1;
}
bool stopWaitRecv(message& recv, message send)								//服务端停等机制
{
	while (1)
	{
		Recv(recv);
		if (recv.checksum())
		{
			send.set_ack(recv);
			send.msgseq = seq++;
			Send(send);
			memset((char*)&send, 0, sizeof(message));
			return true;
		}
	}
	cout << "接收失败" << endl;
	return false;
}
void write(char name[32],int length,int &index)
{
	ofstream write(name, ofstream::binary);
	for (int i = 0; i < index; i++)
	{
		for (int j = 0; j < 1024; j++)
			write << temp[i][j];
	}
	for (int j = 0; j < length; j++)
		write << temp[index][j];
	write.close();
}
int recvFile(message recv)
{
	message send= message();
	send.set_ack(recv);
	send.msgseq = seq++;
	Send(send);
	int index = recv.index;
	int length = recv.length;
	char filename[32];
	memset(filename, 0, sizeof(filename));
	for (int i = 0; recv.msg[i]; i++)
		filename[i] = recv.msg[i];
	for (int i = 0; i <= index; i++)
	{
		message recv, send;
		memset(temp[i], 0, 1024);
		if (stopWaitRecv(recv, send))
		{
			if (i == index)
			{
				for (int j = 0; j < length; j++)
					temp[i][j] = recv.msg[j];
			}
			else
			{
				for (int j = 0; j < 1024; j++)
					temp[i][j] = recv.msg[j];
			}
		}
		else
		{
			cout << "接受失败！" << endl;
			return 0;
		}
		if (i == index)
		{
			if (!recv.get_endfile())
			{
				cout << "消息接收失败！" << endl;
				return 0;
			}
		}
	}	
	write(filename, length, index);
	cout << filename << "文件接收成功" << endl;
	return 1;
}

int main()
{
	WORD wVersionRequested = MAKEWORD(2, 2);//套接字库版本号
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	serverSock = socket(AF_INET, SOCK_DGRAM, 0);
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3484);
	//clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");					//绑定客户端ip地址
	//clientAddr.sin_family = AF_INET;
	//clientAddr.sin_port = htons(3483);
	bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR));
	while (1)
	{
	
		message recv;
		Recv(recv);
		if (recv.get_syn())
		{
			cout << "建立链接..." << endl;
			if (buildConnection(recv))
			{
				

			}
			else
			{
				cout << "链接失败！" << endl;
			}
		}
		else if (recv.get_start())
		{
			cout << "接收文件中..." << endl;
			recvFile(recv);
		}
		if (recv.get_fin())
		{
			if (disconnect(recv))
			{
				seq = 0;
				cout << "客户端链接已断开，输入exit()退出" << endl;
				string exit;
				cin >> exit;
				if (exit == "exit()")
					return 0;
			}
		}
	}
	
}
