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
		cout << "创建socket失败!!!" << endl;
		WSACleanup();
		return -1;
	}
	SOCKADDR_IN serverAddr;
	cout << "请输入你的昵称：" << endl;
	cin >> clientName;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	cout << "等待连接..." << endl;
	if (SOCKET_ERROR == connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
	{
		cout << "连接主机失败，程序即将退出!!!" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}
	else
	{
		cout << "连接成功！" << endl;
	}
	char buffer[1024] = { 0 };

	//启动线程
	_beginthread(threadReceive, 0, NULL);
	send(clientSocket, clientName, sizeof(clientName),0);

	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		cout << "My：";
		cin.getline(buffer,sizeof(buffer));
		if (str_cmp(buffer, "exit()") == 0)
		{
			cout << "程序即将退出..." << endl;
			closesocket(clientSocket);
		}
		send(clientSocket, buffer, sizeof(buffer), 0);
	}
	closesocket(clientSocket);
	WSACleanup();
	cout << "结束通信" << endl;
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
		cout << endl << otherClientName << "：" << buffer;
		cout << "   " << sys.wYear << "/" << sys.wMonth << "/" << sys.wDay << "/  " << sys.wHour << ":" << sys.wMinute << endl;
		cout << "My：";
	}
}