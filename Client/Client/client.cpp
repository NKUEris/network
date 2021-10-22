#include<iostream>
#include<winsock.h>
#include<windows.h>
#include<string>
#include<string.h>
#include<process.h>
#include<time.h>
# pragma comment(lib,"ws2_32.lib")
#define TIME_MAX 64

using namespace std;

void threadReceive(PVOID param);
SOCKET clientSocket = INVALID_SOCKET;
char clientName[128] = { 0 };
char bufferGlobal[1024] = { 0 };
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
	

	//启动线程
	_beginthread(threadReceive, 0, NULL);
	send(clientSocket, clientName, sizeof(clientName),0);

	while (1)
	{
		memset(bufferGlobal, 0, sizeof(bufferGlobal));
		cout << "My：";
		cin>>bufferGlobal;
		if (strcmp(bufferGlobal, "exit()") == 0)
		{
			cout << "程序即将退出..." << endl;
			send(clientSocket, bufferGlobal, sizeof(bufferGlobal), 0);
			/*closesocket(clientSocket);*/
			/*WSACleanup();*/
			cout << "通信结束" << endl;
			return 0;
		}
			send(clientSocket, bufferGlobal, sizeof(bufferGlobal), 0);
			time_t now;
			time(&now);
			struct tm tmTmp;
			char strTmp[TIME_MAX];
			localtime_s(&tmTmp, &now);

			//转化为字符串
			asctime_s(strTmp, &tmTmp);
			cout << strTmp;
	}
	/*closesocket(clientSocket);*/
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
	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		if(recv(clientSocket, buffer, sizeof(buffer), 0)!=0&&bufferGlobal!="exit()");
		{
			if (strcmp(buffer, "exit()") == 0)
			{
				cout <<endl<< "对方结束通信，请输入“exit()”退出程序" << endl<<"My：";

				/*closesocket(clientSocket);*/
				WSACleanup();
				memset(buffer, 0, sizeof(buffer));
				_endthread();
				return ;
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

				if (bufferGlobal != "exit()")
				{
					cout << endl << otherClientName << "：" << buffer << "   " << strTmp << "My：";
				}
			}
		}
	}
}