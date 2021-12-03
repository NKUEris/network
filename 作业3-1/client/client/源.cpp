#include<iostream>
#include"Winsock2.h"
#include<fstream>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)
int seq = 0;
using namespace std;
SOCKET clientSock=INVALID_SOCKET;
SOCKADDR_IN clientAddr;
SOCKADDR_IN serverAddr;
int addrlen = sizeof(SOCKADDR);
char temp[10000][1024];

class message
{
public:
	unsigned short flag=0x01;															// 32标志位 0X01 SYN,0X02 ACK, 0X04 FIN, 0X08 文件开始传输，0XF 文件传输结束
	DWORD DstIP = inet_addr("127.0.0.1"), SrcIP = inet_addr("127.0.0.1");														//源ip和目标ip64
	u_short DstPort = 3484, SrcPort = 3483;										//源端口号和目标端口号32
	int msgseq=0;																//消息序号32
	int ackseq=0;																//接收ack的消息序号32
	int length=0;																//定长报文长度32
	int index=0;																//描述消息条数32
	unsigned short check=0;																//校验和16
	char msg[1024] = { '0' };															//用于存储消息内容
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
	void set_fin();
	int get_fin();
	int checksum();
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
	/*unsigned long checksum = 0;
	unsigned short* buf = (unsigned short*)this;
	for (int i = 0; i < 528; i++)
	{
		checksum += buf[i];
		int t = checksum >> 16;
		checksum = t + (checksum & 0xffff);
	}
	if (checksum & 0xffff == (0xffff))
		return 1;
	return 0;*/
	unsigned short sum = 0;
	u_char* temp = (u_char*)this;
	for (int i = 0; i < 528; i++)
	{
		sum += temp[2 * i] << 8 + temp[2 * i + 1];
		while (sum >= 0x10000)					//溢出
		{
			unsigned short t = sum >> 16;//将最高位回滚添加至最低位
			sum += t;
		}
	}
	if (check + sum == 0xffff)return 1;
	else return 0;
}
void message::setcheck()		//发送方校验和
{
	/*unsigned long checksum = 0;
	unsigned short* buf = (unsigned short*)this;
	for (int i = 0; i < 528; i++)
	{
		checksum += buf[i];
		int t = checksum >> 16;
		checksum = t + (checksum & 0xffff);
	}
	this->check = ~checksum & 0xffff;*/
	unsigned short sum = 0;
	u_char* temp = (u_char*)this;
	for (int i = 0; i < 528; i++)
	{
		sum += temp[2 * i] << 8 + temp[2 * i + 1];
		while (sum >=0x10000)					//溢出
		{
			unsigned short t = sum >> 16;//将最高位回滚添加至最低位
			sum += t;
		}
	}
	this->check = (unsigned short)(~sum);				//按位取反
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

void Send(message& Msg)
{
	Msg.setcheck();
	Msg.msgseq = seq++;
	sendto(clientSock, (char*)&Msg, sizeof(message), 0, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR));
}
void Recv(message& Msg)
{
	memset(Msg.msg, 0, sizeof(Msg.msg));
	recvfrom(clientSock, (char*)&Msg, sizeof(message), 0, (SOCKADDR*)&serverAddr, (int*)&addrlen);
	seq = Msg.msgseq;
}
bool stopWaitSend(message& send, message recv)
{
	Send(send);
	clock_t start = clock();
	int count = 0;						//重发计数器
	while (1)
	{
		Recv(recv);						//接收消息
		if (recv.get_ack() && recv.ackseq == send.msgseq)
		{
			//收到消息确认
			return true;
		}
		clock_t end = clock();
		if (count == 5)				//重新发送次数超过五次
		{
			return false;
		}
		if ((double)((end - start) / CLOCKS_PER_SEC) >= 3)
		{
			count++;
			start = clock();			//重新计时
			cout << "发送失败，正在尝试第" << count << "次重新发送" << endl;
			Send(send);
		}
	}
}
int buildConnection()
{
	message send = message();
	message recv = message();
	send.set_syn();
	send.msgseq = seq++;
	if (stopWaitSend(send, recv))return 1;
	else return 0;
}
int disconnect()
{
	message send = message();
	message recv = message();
	send.set_fin();
	send.msgseq = seq++;
	if (stopWaitSend(send, recv))return 1;
	else return 0;

}

int read(char name[32], int &length, int &index)
{
	index = 0;
	length = 0;
	memset(temp, 0, sizeof(temp));
	ifstream sendfile(name, ifstream::binary);
	if (!sendfile)
	{
		cout << "文件打开失败...." << endl;
		return 0;
	}
	char file = sendfile.get();
	while (sendfile) 
	{
		temp[index][length % 1024] = file;
		length++;
		if (length % 1024 == 0)
		{
			index++;
			length = 0;
		}
		file = sendfile.get();
	}
	sendfile.close();
	return 1;
}
int sendfile(char name[32])
{
	int index = 0;
	int length = 0;
	read(name, length, index);
	clock_t start = clock();
	message send;
	send.index = index;
	send.length = length;
	for (int i = 0; i < strlen(name); i++)
	{
		send.msg[i] = name[i];
	}
	send.set_sart();
	send.msgseq = seq++;
	message recv;
	if (!stopWaitSend(send, recv))
	{
		cout << "文件发送失败..." << endl;
		return 0;
	}
	for (int i = 0; i <= index; i++)
	{
		message end;
		if (i == index)
		{
			end.set_end();
			for (int j = 0; j < length; j++)
			{
				end.msg[j] = temp[index][j];
			}
		}
		else
		{
			for (int j = 0; j < 1024; j++)
			{
				end.msg[j] = temp[i][j];
			}	
		}
		end.msgseq = seq++;
		if (!stopWaitSend(end, recv))
		{
			cout << "文件发送失败..." << endl;
			return 0;
		}
	}
	clock_t timeend = clock();
	double time = ((double)(timeend - start) / CLOCKS_PER_SEC);
	cout << "传输时间：" << time << "s" << endl;
	cout<<"吞吐率：" << (double)(index + 1) * sizeof(message) * 8 / time/ 1024 / 1024 << "Mbps" << endl;
	cout << "文件传输成功！" << endl;
	return 1;
}
int main()
{
	WORD wVersionRequested = MAKEWORD(2, 2);//套接字库版本号
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	clientSock = socket(AF_INET, SOCK_DGRAM, 0);
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3484);
	char op;
	cout << "是否需要传输文件？(Y/N)" << endl;
	while (1)
	{
		cin >> op;
		while (op == 'Y')
		{
			cout << "请输入要发送的文件名: " << endl;
			char name[32];
			cin >> name;
			if (buildConnection())
			{
				cout << "建立链接成功！" << endl;
				cout << "准备发送文件" << endl;
				if (sendfile(name))
				{
					cout << "文件发送成功！" << endl;
					cout << "是否需要继续发送文件？(Y/N)" << endl;
					cin >> op;
				}
				else
					cout << "文件发送失败！" << endl;
			}
			else
			{
				cout << "建立链接失败..." << endl;
				return 0;
			}
		}
		if (disconnect())
		{
			cout << "链接已断开！" << endl;
		}
		else
		{
			cout << "断开链接失败！" << endl;
		}

		return 0;
	}
}

