#include<iostream>
#include<winsock.h>
#include<string>
# pragma comment(lib,"ws2_32.lib")
using namespace std;
int main()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//创建套接字
	SOCKET clientSocket;
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "创建socket失败!!!" << endl;
		WSACleanup();
		return 0;
	}
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	unsigned port = 3484;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	char clientName[128] = { 0 };
	cout << "请输入你的名字：";
	cin.getline(clientName, 128);
	connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	cout << "连接成功" << endl;

	char serverName[128] = { 0 };
	char buffer[1024] = { 0 };
	send(clientSocket, clientName, sizeof(clientName), 0);
	recv(clientSocket, serverName, sizeof(serverName), 0);

	while (1)
	{
		while (1)
		{
			
			cout << clientName << "：";
			memset(buffer, 0, sizeof(buffer));
			cin >> buffer;
			if (strcmp(buffer, "exit()") == 0)
			{
				cout << "程序即将退出" << endl;
				closesocket(clientSocket);
				WSACleanup();
				return 0;
			}
			send(clientSocket, buffer, sizeof(buffer), 0);
		}
		while (1)
		{
			memset(buffer, 0, sizeof(buffer));
			recv(clientSocket, buffer, sizeof(buffer), 0);
			if (strcmp(buffer, "exit()") == 0)
			{
				cout << "程序即将退出" << endl;
				closesocket(clientSocket);
				WSACleanup();
				return 0;
			}
			cout << serverName << "：" << buffer << endl;
		}
	}
	
}