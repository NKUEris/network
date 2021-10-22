#include<iostream>
#include<winsock.h>
#include<string>
#include<process.h>
# pragma comment(lib,"ws2_32.lib")
using namespace std;

void threadReceive(PVOID param);
SOCKET clientSocket = INVALID_SOCKET;
char clientName[128] = { 0 };
//int main()
//{
//	WSADATA wsaData;
//	WSAStartup(MAKEWORD(2, 2), &wsaData);
//	//创建套接字
//	SOCKET clientSocket;
//	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (clientSocket == INVALID_SOCKET)
//	{
//		cout << "创建socket失败!!!" << endl;
//		WSACleanup();
//		return 0;
//	}
//	SOCKADDR_IN serverAddr;
//	serverAddr.sin_family = AF_INET;
//	unsigned port = 3484;
//	serverAddr.sin_port = htons(port);
//	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
//	char clientName[128] = { 0 };
//	cout << "请输入你的名字：";
//	cin.getline(clientName, 128);
//	connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
//	cout << "连接成功" << endl;
//
//	char serverName[128] = { 0 };
//	char buffer[1024] = { 0 };
//	send(clientSocket, clientName, sizeof(clientName), 0);
//	recv(clientSocket, serverName, sizeof(serverName), 0);
//
//	while (1)
//	{
//		while (1)
//		{
//			
//			cout << clientName << "：";
//			memset(buffer, 0, sizeof(buffer));
//			cin >> buffer;
//			if (strcmp(buffer, "exit()") == 0)
//			{
//				cout << "程序即将退出" << endl;
//				closesocket(clientSocket);
//				WSACleanup();
//				return 0;
//			}
//			send(clientSocket, buffer, sizeof(buffer), 0);
//		}
//		while (1)
//		{
//			memset(buffer, 0, sizeof(buffer));
//			recv(clientSocket, buffer, sizeof(buffer), 0);
//			if (strcmp(buffer, "exit()") == 0)
//			{
//				cout << "程序即将退出" << endl;
//				closesocket(clientSocket);
//				WSACleanup();
//				return 0;
//			}
//			cout << serverName << "：" << buffer << endl;
//		}
//	}
//	
//}
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
		cin >> buffer;
		send(clientSocket, buffer, sizeof(buffer), 0);
	}
	closesocket(clientSocket);
	WSACleanup();
	cout << "结束通信" << endl;
	return 0;


}
void threadReceive(PVOID param)
{
	char buffer[1024] = { 0 };
	char otherClientName[128] = { 0 };
	recv(clientSocket, otherClientName, sizeof(otherClientName), 0);
	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		recv(clientSocket, buffer, sizeof(buffer), 0);
		cout << endl<< otherClientName << "：" << buffer << endl;
	}
}