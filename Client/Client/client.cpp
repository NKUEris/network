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
	if (clientSocket = INVALID_SOCKET)
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
	return 0;
}