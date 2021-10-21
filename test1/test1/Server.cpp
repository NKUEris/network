#include<iostream>
#include<string>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")						//����ws2_32.lib��
using namespace std;
void serverInit();

int main()
{
	WSADATA	wsaData;									//���socket
	WSAStartup(MAKEWORD(2, 2), &wsaData);				//����socket�汾
	SOCKET serverSocket;								//����socket
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);		// //��ַ����ΪAD_INET����������Ϊ��ʽ(SOCK_STREAM)��Э�����TCP  
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "����socketʧ��!!!" << endl;
		WSACleanup();
		return 0;
	}

	/* struct SOCKADDR_IN{
	* sin_family;        //Address Family
	* sin_port;			//16λ�˿ں�
	* sin_addr;			//32λ��ַ
	* sin_zero[8];		//��ʹ��
	*/
	unsigned port = 3484;								//�˿ں�
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;		
	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)  //�������� 
	{
		cout << "�󶨶˿�ʧ��!!!" << endl;
		closesocket(serverSocket);
		return 0;
	}
	//����server����
	char serverName[128] = { 0 };
	cout << "�������������֣�" << endl;
	cin.getline(serverName, 128);

	//��ʼ����
	listen(serverSocket, 5);

	//�ȴ�����
	char clientName[128] = { 0 };
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	cout << "�ȴ�����......" << endl;
	clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrlen);
	if (clientSocket == INVALID_SOCKET)
	{
		cout << "�ͻ�������ʧ��!!!" << endl;
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}
	else
	{
		cout << "�ͻ������ӳɹ�!" << endl;
	}

	//������buffer
	char buffer[1024] = { 0 };

	//��ʼͨ��
	//���ͺͽ��ܿͻ���������������
	send(clientSocket, serverName, sizeof(serverName), 0);
	recv(clientSocket, clientName, sizeof(clientName), 0);
	while (1)
	{
		while (1)
		{
			memset(buffer, 0, sizeof(buffer));					//���ݽ���֮ǰ��ջ���������
			recv(clientSocket, buffer, sizeof(buffer), 0);
			if (strcmp(buffer, "exit()") == 0)					//����˳�����Ĺؼ���
			{
				cout << "���򼴽��˳�" << endl;
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
			cin >> buffer;
			if (strcmp(buffer, "exit()") == 0)					//����˳�����Ĺؼ���
			{
				cout << "���򼴽��˳�" << endl;
				closesocket(serverSocket);
				closesocket(clientSocket);
				WSACleanup();
				return 0;
			}
			send(clientSocket, buffer, sizeof(buffer), 0);
		}

	}

}