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
	unsigned short flag=0x01;															// 32��־λ 0X01 SYN,0X02 ACK, 0X04 FIN, 0X08 �ļ���ʼ���䣬0XF �ļ��������
	DWORD DstIP = inet_addr("127.0.0.1"), SrcIP = inet_addr("127.0.0.1");														//Դip��Ŀ��ip64
	u_short DstPort = 3484, SrcPort = 3483;										//Դ�˿ںź�Ŀ��˿ں�32
	int msgseq=0;																//��Ϣ���32
	int ackseq=0;																//����ack����Ϣ���32
	int length=0;																//�������ĳ���32
	int index=0;																//������Ϣ����32
	unsigned short check=0;																//У���16
	char msg[1024] = { '0' };															//���ڴ洢��Ϣ����
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
		while (sum >= 0x10000)					//���
		{
			unsigned short t = sum >> 16;//�����λ�ع���������λ
			sum += t;
		}
	}
	if (check + sum == 0xffff)return 1;
	else return 0;
}
void message::setcheck()		//���ͷ�У���
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
		while (sum >=0x10000)					//���
		{
			unsigned short t = sum >> 16;//�����λ�ع���������λ
			sum += t;
		}
	}
	this->check = (unsigned short)(~sum);				//��λȡ��
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
		cout << "�ļ���ʧ��...." << endl;
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
		cout << "�ļ�����ʧ��..." << endl;
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
			cout << "�ļ�����ʧ��..." << endl;
			return 0;
		}
	}
	clock_t timeend = clock();
	double time = ((double)(timeend - start) / CLOCKS_PER_SEC);
	cout << "����ʱ�䣺" << time << "s" << endl;
	cout<<"�����ʣ�" << (double)(index + 1) * sizeof(message) * 8 / time/ 1024 / 1024 << "Mbps" << endl;
	cout << "�ļ�����ɹ���" << endl;
	return 1;
}
int main()
{
	WORD wVersionRequested = MAKEWORD(2, 2);//�׽��ֿ�汾��
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	clientSock = socket(AF_INET, SOCK_DGRAM, 0);
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3484);
	char op;
	cout << "�Ƿ���Ҫ�����ļ���(Y/N)" << endl;
	while (1)
	{
		cin >> op;
		while (op == 'Y')
		{
			cout << "������Ҫ���͵��ļ���: " << endl;
			char name[32];
			cin >> name;
			if (buildConnection())
			{
				cout << "�������ӳɹ���" << endl;
				cout << "׼�������ļ�" << endl;
				if (sendfile(name))
				{
					cout << "�ļ����ͳɹ���" << endl;
					cout << "�Ƿ���Ҫ���������ļ���(Y/N)" << endl;
					cin >> op;
				}
				else
					cout << "�ļ�����ʧ�ܣ�" << endl;
			}
			else
			{
				cout << "��������ʧ��..." << endl;
				return 0;
			}
		}
		if (disconnect())
		{
			cout << "�����ѶϿ���" << endl;
		}
		else
		{
			cout << "�Ͽ�����ʧ�ܣ�" << endl;
		}

		return 0;
	}
}

