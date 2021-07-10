#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

vector <string> tmpChats;

void saveChat(vector <string> v)
{
	ofstream file;
	file.open("chatlog.txt");
	for (int i = 0; i < v.size(); ++i)
	{
		file << v[i] << endl;
	}
	file.close();
}

int main()
{
	int port = 54000;

	// Initialize Winsock
	WSADATA wsData; // WinSock data-structure
	WORD ver = MAKEWORD(2, 2); // means 2.2 version

	int wsOk = WSAStartup(ver, &wsData); // initializing Winsock lib 
	cout << "WinSock Started!!" << endl;

	if (wsOk != 0)
	{
		cout << "Can`t Initialize Winsock! Quitting" << endl;
		return 1;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0); // make socket with IPv4, TCP (SOCK_DGRAM:UDP)
	cout << "Socket Created" << endl;

	if (listening == INVALID_SOCKET)
	{
		cout << "Can`t create a socket! Quitting" << endl;
		return 1;
	}

	// Bind the socket to an ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port); // host byte to network byte (little->big endian)
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening
	listen(listening, SOMAXCONN);

	fd_set master; // data-structrue that contains list of socket, and their stats
	FD_ZERO(&master);
	//cout << master.fd_array[0] << endl;
	FD_SET(listening, &master);

	bool running = true;
	
	while (running)
	{
		// select function cause chaning of fd. so we need copy of fd
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				//cout << listening;
				// Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				string welcomeMsg = "Welcome to the Chat Server!\r\n";
				//cin >> welcomeMsg;
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

				// broadcast welcome message
				ostringstream ss;
				ss << "Client" << " #" << client << " connect to chat server.\r\n";
				string strOut = ss.str();
				for (int i = 0; i < master.fd_count; i++)
				{
					SOCKET outSock = master.fd_array[i];
					if (outSock != listening && outSock != client)
					{
						send(outSock, strOut.c_str(), strOut.size() + 1, 0);
					}
				}
				// logging welcome message to server
				cout << strOut << endl;
			}
			else
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive Message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// Check to see if it`s a command.  \quit kills the server
					if (buf[0] == '\\')
					{
						string cmd = string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							running = false;
							break;
						}						
					}

					// Send message to other client, and definiately not the listening socket
					ostringstream ss;
					ss << "Client #" << sock << ":" << buf << "\r\n";
					string strOut = ss.str();
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						//cout << outSock << " A ";
						if (outSock != listening && sock != outSock)
						{
							send(outSock, strOut.c_str(), strOut.size()+1, 0);
							//cout << outSock << " A ";
						}
					}
					// logging user`s message to server
					cout << strOut << endl;

					tmpChats.push_back(strOut);
					saveChat(tmpChats);
				}
			}
		}
	}


	// Close listening socket
	FD_CLR(listening, &master);
	closesocket(listening);

	// Cleanup Winsock
	WSACleanup();
	return 0;
}


