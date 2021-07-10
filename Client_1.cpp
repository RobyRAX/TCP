#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <thread>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

void recv_socket(SOCKET sock);

int main()
{
	string ipAddress = "127.0.0.1";	// IP Address of the server
	int port = 54000;				// Listening port number on the server

	// Initialize Winsock
	WSAData data; // WinSock data-structure
	WORD ver = MAKEWORD(2, 2); // 2.2 version

	int wsResult = WSAStartup(ver, &data); //init winsock lib
	if (wsResult != 0)
	{
		cout << "Can`t start Winsock, Err #" << wsResult << endl;
		return 1;
	}

	// Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cout << "Can`t create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	// Fill in a hint structre
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cout << "Can`t connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	// thread to receive data
	thread recv_thread(recv_socket, sock);

	// Do-while loop to send data
	string userInput;
	do
	{
		// Prompt the user for some text
		getline(cin, userInput);
		if (userInput.size() > 0) // Make sure the user has typed in something
		{
			// Send the text
			send(sock, userInput.c_str(), userInput.size() + 1, 0);
		}
	} while (userInput.size() > 0);
	// Gracefully close down everything
	recv_thread.join();
	closesocket(sock);
	WSACleanup();
	return 0;
}
void recv_socket(SOCKET sock)
{
	char buf[4096];
	while (1)
	{
		ZeroMemory(buf, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);
		if (bytesReceived > 0)
		{
			cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
		}
	}
}

