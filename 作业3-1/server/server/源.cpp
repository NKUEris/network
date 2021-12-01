#include<iostream>
#include"Winsock2.h"
#include<time.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
SOCKET serverSock = INVALID_SOCKET;
SOCKADDR_IN clientAddr;
SOCKADDR_IN serverAddr;
int addrlen = sizeof(SOCKADDR);
int seq = 0;
#pragma pack(1)
class message
{
public:
	int flag;														//��־λ 0X01 SYN,0X02 ACK, 0X04 FIN, 0X08 �ļ���ʼ���䣬0XF �ļ��������
	DWORD DstIP, SrcIP;														//Դip��Ŀ��ip
	u_short DstPort, SrcPort;												//Դ�˿ںź�Ŀ��˿ں�
	int msgseq;																//��Ϣ���
	int ackseq;																//����ack����Ϣ���
	int length;																//�������ĳ���
	int index;																//������Ϣ����
	int check;																//У���
	int fill;																//���
	char msg[1024];															//���ڴ洢��Ϣ����
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
	if (this->flag & 0x20)
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
	int sum = 0;
	u_char* temp = (u_char*)this;
	for (int i = 0; i < 16; i++)
	{
		sum += temp[2 * i] << 8 + temp[2 * i + 1];
		while (sum > 0x10000)					//���
		{
			int cont = sum >> 16;
			sum += cont;
		}
	}
	if (check + (u_short)sum == 0xffff)return 1;
	else return 0;
}
void message::setcheck()		//���ͷ�У���
{
	int sum = 0;
	u_char* temp = (u_char*)this;
	for (int i = 0; i < 16; i++)
	{
		sum += temp[2 * i] << 8 + temp[2 * i + 1];
		while (sum > 0x10000)					//���
		{
			int cont = sum >> 16;
			sum += cont;
		}
	}
	this->check = ~(u_short)sum;			//��λȡ��
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
#pragma pack()
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
			memset((char*)&send, 0, sizeof(message));
			return true;
		}
	}
	cout << "����ʧ��" << endl;
	return false;
}
void recvFile(message recv)
{
	message send= message();
	send.set_ack(recv);
	send.msgseq = seq++;
	Send(send);
	int index = recv.index;
	int length = recv.length;
	char filename[128];
	memset(filename, 0, sizeof(filename));
	for (int i = 0; recv.msg[i]; i++)
		filename[i] = recv.msg[i];
	for (int i = 0; i <= index; i++)
	{

	}
}
void Send(message& Msg)
{
	Msg.setcheck();
	sendto(serverSock, (char*)&Msg, sizeof(message), 0, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR));
}
void Recv(message& Msg)
{
	memset(Msg.msg, 0, sizeof(Msg.msg));
	recvfrom(serverSock, (char*)&Msg, sizeof(message), 0, (SOCKADDR*)&serverAddr, (int*)&addrlen);
}
int main()
{
	WORD wVersionRequested = MAKEWORD(2, 2);//�׽��ֿ�汾��
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	serverSock = socket(AF_INET, SOCK_DGRAM, 0);
	clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");					//�󶨿ͻ���ip��ַ
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(3484);
	bind(serverSock, (SOCKADDR*)&clientAddr, sizeof(SOCKADDR));

}
