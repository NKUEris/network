#include<iostream>
#include<string>
#include<windows.h>
#include<winsock.h>
#include<process.h>
#include<time.h>
#pragma comment(lib,"ws2_32.lib")						//加载ws2_32.lib库

#define TIME_MAX 64
using namespace std;

typedef struct serverClient
{
	SOCKET serverSocket;
	SOCKET clientSocket1;
	SOCKET clientSocket2;
}CS;
//服务端初始化

void init(CS* cs);
void createSocket(CS* cs);								//创建套接字
void threadStart(CS* cs);								//调用线程
unsigned __stdcall threadReceive1(void* sc);
unsigned __stdcall threadReceive2(void* sc);

char buffer1[1024] = { 0 };
char buffer2[1024] = { 0 };

int main()
{
	CS Cs;
	CS* cs = &Cs;
	
		init(cs);
		createSocket(cs);
		threadStart(cs);
	
	closesocket(cs->serverSocket);
	closesocket(cs->clientSocket1);
	closesocket(cs->clientSocket2);
	WSACleanup();
	return 0;
}
void init(CS* cs)
{
	WSADATA wsaData;														//存放socket
	WSAStartup(MAKEWORD(2, 2), &wsaData);									//声明socket版本
	cs->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (cs->serverSocket == INVALID_SOCKET)
	{
		cout << "创建socket失败!!!" << endl;
		WSACleanup();
		return;
	}
	unsigned port = 3484;													//端口号
		/* struct SOCKADDR_IN{
	* sin_family;        //Address Family
	* sin_port;			//16位端口号
	* sin_addr;			//32位地址
	* sin_zero[8];		//不使用
	*/
	SOCKADDR_IN	serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);										//服务器监听端口
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(cs->serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
	{
		cout << "绑定端口失败!!!" << endl;
		closesocket(cs->serverSocket);
		WSACleanup();
		return;
	}
	listen(cs->serverSocket, 5);
}
void createSocket(CS* cs)
{
	SOCKADDR_IN clientAddr1;
	SOCKADDR_IN clientAddr2;

	int clientAddr1len = sizeof(clientAddr1);
	int clientAddr2len = sizeof(clientAddr2);
	cout << "等待连接..." << endl;
	cs->clientSocket1 = accept(cs->serverSocket, (SOCKADDR*)&clientAddr1, &clientAddr1len);
	cs->clientSocket2 = accept(cs->serverSocket, (SOCKADDR*)&clientAddr2, &clientAddr2len);
	cout << "客户端连接成功。" << endl;
}
void threadStart(CS* cs)
{
	HANDLE thread1;
	HANDLE thread2;
	unsigned threadId1;
	unsigned threadId2;
	cout << "连接成功，开始聊天..." << endl;
	thread1 = (HANDLE)_beginthreadex(NULL, 0, &threadReceive1, cs, 0, &threadId1);
	thread2 = (HANDLE)_beginthreadex(NULL, 0, &threadReceive2, cs, 0, &threadId2);
	WaitForSingleObject(thread1, INFINITE);
	WaitForSingleObject(thread2, INFINITE);

	CloseHandle(thread1);
	CloseHandle(thread2);
}

//对应客户端1的线程
unsigned __stdcall threadReceive1(void* sc)
{
	CS* cs = (CS*)sc;
	char clientName[128] = { 0 };
	recv(cs->clientSocket1, clientName, sizeof(clientName), 0);
	send(cs->clientSocket2, clientName, sizeof(clientName), 0);
	while (1)
	{
		memset(buffer1, 0, sizeof(buffer1));

		if (recv(cs->clientSocket1, buffer1, sizeof(buffer1), 0) != 0)
		{
			if (strcmp(buffer1, "exit()") == 0)
			{
				cout << "程序即将退出..." << endl;
				send(cs->clientSocket2, buffer1, sizeof(buffer1), 0);
				/*closesocket(cs->serverSocket);
				closesocket(cs->clientSocket1);*/
				ExitThread(0);
				break;
			}
			else
			{
				time_t now;
				time(&now);
				struct tm tmTmp;
				char strTmp[TIME_MAX];
				localtime_s(&tmTmp, &now);

				//转化为字符串
				asctime_s(strTmp, &tmTmp);
				cout << clientName << "：" << buffer1 <<"   "<<strTmp<< endl;
				send(cs->clientSocket2, buffer1, sizeof(buffer1), 0);
			}
		}
	}
	return 0;
}

//对应客户端2的线程
unsigned __stdcall threadReceive2(void* sc)
{
	CS* cs = (CS*)sc;
	char clientName[128] = { 0 };
	recv(cs->clientSocket2, clientName, sizeof(clientName), 0);
	send(cs->clientSocket1, clientName, sizeof(clientName), 0);
	while (1)
	{
		//清空缓存区
		memset(buffer2, 0, sizeof(buffer2));
	
		if (recv(cs->clientSocket2, buffer2, sizeof(buffer2), 0) != 0)
		{
			if (strcmp(buffer2, "exit()") == 0)
			{
				cout << "程序即将退出..." << endl;
				send(cs->clientSocket1, buffer2, sizeof(buffer2), 0);
				/*closesocket(cs->serverSocket);
				closesocket(cs->clientSocket1);
				closesocket(cs->clientSocket2);*/
				ExitThread(0);
				return 0;
			}
			else
			{
				time_t now;
				time(&now);
				struct tm tmTmp;
				char strTmp[TIME_MAX];
				localtime_s(&tmTmp, &now);

				//转化为字符串
				asctime_s(strTmp, &tmTmp);
				cout << clientName << "：" << buffer2 <<"   "<<strTmp<< endl;
				send(cs->clientSocket1, buffer2, sizeof(buffer2), 0);
			}
		}
	}
	return 0;
}

