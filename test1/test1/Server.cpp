#include<iostream>
#include<string>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")						//加载ws2_32.lib库
using namespace std;
void serverInit();

int main()
{
	WSADATA	wsaData;									//存放socket
	WSAStartup(MAKEWORD(2, 2), &wsaData);				//声明socket版本
	SOCKET serverSocket;								//创建socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		// //地址类型为AD_INET，服务类型为流式(SOCK_STREAM)，协议采用TCP  
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "创建socket失败!!!" << endl;
		WSACleanup();
		return 0;
	}

	/* struct SOCKADDR_IN{
	* sin_family;        //Address Family
	* sin_port;			//16位端口号
	* sin_addr;			//32位地址
	* sin_zero[8];		//不使用
	*/
	unsigned port = 3484;								//端口号
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htonl(port);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;		
	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)  //建立捆绑 
	{
		cout << "绑定端口失败!!!" << endl;
		closesocket(serverSocket);
		return 0;
	}
	//定义server名字
	char serverName[128] = { 0 };
	cout << "请输入主机名字：" << endl;
	cin.getline(serverName, 128);

	//开始监听
	listen(serverSocket, 5);

	//等待连接
	char clientName[128] = { 0 };
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrlen);
	cout << "等待连接......" << endl;
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "客户端连接失败!!!" << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}
	else
	{
		cout << "客户端连接成功!" << endl;
	}

	//缓冲区buffer
	char buffer[1024] = { 0 };

	//开始通信
	//发送和接受客户端与主机的名字
	send(clientSocket, serverName, sizeof(serverName), 0);
	recv(clientSocket, clientName, sizeof(clientName), 0);
	while (1)
	{
		while (1)
		{
			memset(buffer, 0, sizeof(buffer));					//数据交互之前清空缓冲区数据
			recv(clientSocket, buffer, sizeof(buffer), 0);
			if (strcmp(buffer, "exit()") == 0)					//检测退出程序的关键词
			{
				cout << "程序即将退出" << endl;
				closesocket(serverSocket);
				closesocket(clientSocket);
				WSACleanup();
				return 0;
			}
			cout << clientName << ":" << buffer << endl;
		}
		while (1)
		{
			memset(buffer, 0, sizeof(buffer));
			cout << serverName << ": ";
			cin.getline(buffer, 1024);
			if (strcmp(buffer, "exit()") == 0)					//检测退出程序的关键词
			{
				cout << "程序即将退出" << endl;
				closesocket(serverSocket);
				closesocket(clientSocket);
				WSACleanup();
				return 0;
			}
			send(clientSocket, buffer, sizeof(buffer), 0);
		}

	}

}