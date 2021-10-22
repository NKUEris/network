#include<iostream>
#include<winsock.h>
#include<windows.h>
#include<string>
#include<process.h>
# pragma comment(lib,"ws2_32.lib")
using namespace std;

void threadReceive(PVOID param);
SOCKET clientSocket = INVALID_SOCKET;
char clientName[128] = { 0 };

int main()
{
	int port = 3484;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "����socketʧ��!!!" << endl;
		WSACleanup();
		return -1;
	}
	SOCKADDR_IN serverAddr;
	cout << "����������ǳƣ�" << endl;
	cin >> clientName;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	cout << "�ȴ�����..." << endl;
	if (SOCKET_ERROR == connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
	{
		cout << "��������ʧ�ܣ����򼴽��˳�!!!" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}
	else
	{
		cout << "���ӳɹ���" << endl;
	}
	char buffer[1024] = { 0 };

	//�����߳�
	_beginthread(threadReceive, 0, NULL);
	send(clientSocket, clientName, sizeof(clientName),0);

	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		cout << "My��";
		cin.getline(buffer,sizeof(buffer));
		if (str_cmp(buffer, "exit()") == 0)
		{
			cout << "���򼴽��˳�..." << endl;
			closesocket(clientSocket);
		}
		send(clientSocket, buffer, sizeof(buffer), 0);
	}
	closesocket(clientSocket);
	WSACleanup();
	cout << "����ͨ��" << endl;
	return 0;
}
void threadReceive(PVOID param)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char buffer[1024] = { 0 };
	char otherClientName[128] = { 0 };
	recv(clientSocket, otherClientName, sizeof(otherClientName), 0);
	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		recv(clientSocket, buffer, sizeof(buffer), 0);
		cout << endl << otherClientName << "��" << buffer;
		cout << "   " << sys.wYear << "/" << sys.wMonth << "/" << sys.wDay << "/  " << sys.wHour << ":" << sys.wMinute << endl;
		cout << "My��";
	}
}