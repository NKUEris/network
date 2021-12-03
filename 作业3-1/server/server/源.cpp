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
	unsigned short flag=0x01;														//��־λ 0X01 SYN,0X02 ACK, 0X04 FIN, 0X08 �ļ���ʼ���䣬0XF �ļ��������
	DWORD DstIP=inet_addr("127.0.0.1"), SrcIP =inet_addr("127.0.0.1");	//Դip��Ŀ��ip
	u_short DstPort=3484, SrcPort=3483;											//Դ�˿ںź�Ŀ��˿ں�
	int msgseq=0;																//��Ϣ���
	int ackseq=0;																//����ack����Ϣ���
	int length=0;																//�������ĳ���
	int index=0;																//������Ϣ����
	unsigned short check=0;																//У���															
	char msg[1024] = {'0'};															//���ڴ洢��Ϣ����
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
	//ACK = 010�ڶ�λ��־λ
	if (this->flag & 0x02)
		return 1;
	else return 0;
}
void message::set_ack(message Msg)		//����Ϣ��ȷ��
{
	if (get_ack() == 0)
		this->flag += 0x02;
	this->ackseq = Msg.msgseq;
}
int message::checksum()				//��У��ͽ��м���
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
	//	while (sum >=0x10000)					//���
	//	{
	//		unsigned t = sum >> 16;//�����λ�ع���������λ
	//		sum += t;
	//	}
	//}
	//if (check + sum == 0xffff)return 1;
	//else return 0;
}
void message::setcheck()		//���ͷ�У���
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
	//	while (sum >= 0x10000)					//���
	//	{
	//		unsigned short t = sum >> 16;//�����λ�ع���������λ
	//		sum += t;
	//	}
	//}
	//this->check = (unsigned short)(~ sum);			//��λȡ��

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
	cout << "���ӳɹ�" << endl;
	return 1;
}
int disconnect(message Msg)
{
	message send = message();
	send.set_ack(Msg);
	send.msgseq = seq++;
	Send(send);
	cout << "�ͻ����Ѿ��Ͽ�����....." << endl;
	return 1;
}
bool stopWaitRecv(message& recv, message send)								//�����ͣ�Ȼ���
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
	cout << "����ʧ��" << endl;
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
			cout << "����ʧ�ܣ�" << endl;
			return 0;
		}
		if (i == index)
		{
			if (!recv.get_endfile())
			{
				cout << "��Ϣ����ʧ�ܣ�" << endl;
				return 0;
			}
		}
	}	
	write(filename, length, index);
	cout << filename << "�ļ����ճɹ�" << endl;
	return 1;
}

int main()
{
	WORD wVersionRequested = MAKEWORD(2, 2);//�׽��ֿ�汾��
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	serverSock = socket(AF_INET, SOCK_DGRAM, 0);
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3484);
	//clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");					//�󶨿ͻ���ip��ַ
	//clientAddr.sin_family = AF_INET;
	//clientAddr.sin_port = htons(3483);
	bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR));
	while (1)
	{
	
		message recv;
		Recv(recv);
		if (recv.get_syn())
		{
			cout << "��������..." << endl;
			if (buildConnection(recv))
			{
				

			}
			else
			{
				cout << "����ʧ�ܣ�" << endl;
			}
		}
		else if (recv.get_start())
		{
			cout << "�����ļ���..." << endl;
			recvFile(recv);
		}
		if (recv.get_fin())
		{
			if (disconnect(recv))
			{
				seq = 0;
				cout << "�ͻ��������ѶϿ�������exit()�˳�" << endl;
				string exit;
				cin >> exit;
				if (exit == "exit()")
					return 0;
			}
		}
	}
	
}
