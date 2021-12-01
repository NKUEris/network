#include<iostream>
#include"Winsock2.h"
#include<time.h>
#pragma comment(lib,"ws2_32.lib")
int seq = 0;
using namespace std;
SOCKET clientSock=INVALID_SOCKET;
SOCKADDR_IN clientAddr;
SOCKADDR_IN serverAddr;
int addrlen = sizeof(SOCKADDR);
#pragma pack(1)
class message
{
public:
	int flag=0x01;															//��־λ 0X01 SYN,0X02 ACK, 0X04 FIN, 0X08 �ļ���ʼ���䣬0XF �ļ��������
	DWORD DstIP, SrcIP;														//Դip��Ŀ��ip
	u_short DstPort, SrcPort;													//Դ�˿ںź�Ŀ��˿ں�
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
	void set_fin();
	int get_fin();
	int checksum();
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
bool stopWaitSend(message& send, message recv)
{
	Send(send);
	clock_t start = clock();
	int count = 0;						//�ط�������
	while (1)
	{
		Recv(recv);						//������Ϣ
		if (recv.get_ack() && recv.ackseq == send.msgseq)
		{
			//�յ���Ϣȷ��
			return true;
		}
		clock_t end = clock();
		if (count == 5)				//���·��ʹ����������
		{
			return false;
		}
		if ((double)((end - start) / CLOCKS_PER_SEC) >= 3)
		{
			count++;
			start = clock();			//���¼�ʱ
			cout << "����ʧ�ܣ����ڳ��Ե�" << count << "�����·���" << endl;
			Send(send);
		}
	}
}

void Send(message& Msg)
{
	Msg.setcheck();
	Msg.msgseq = seq++;
	sendto(clientSock, (char*)&Msg, sizeof(message),0,(SOCKADDR*)&serverAddr, sizeof(SOCKADDR));
}
void Recv(message& Msg)
{
	memset(Msg.msg, 0, sizeof(Msg.msg));
	recvfrom(clientSock, (char*)&Msg, sizeof(message), 0, (SOCKADDR*)&serverAddr,(int*)&addrlen);
	seq = Msg.msgseq;
}
void Read()
{
}
void sendfile()
{

}
int main()
{
	WORD wVersionRequested = MAKEWORD(2, 2);//�׽��ֿ�汾��
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	clientSock = socket(AF_INET, SOCK_DGRAM, 0);
	clientAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");					//�󶨿ͻ���ip��ַ
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(3484);
	bind(clientSock, (SOCKADDR*)&clientAddr, sizeof(SOCKADDR));
	
}
