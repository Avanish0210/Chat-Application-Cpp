#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h> // For sockaddr_in and inet_pton
#include <tchar.h>>
#include <thread>
#include <vector>

using namespace std;
#pragma comment(lib ,"ws2_32.lib") // Link with ws2_32.lib

/*   
      //initialize winsock library
	  //create the socket
	  //get ip and port
	  //bind the ip / port with the socket
	  //listen on the socket
	  //accept
	  //recv and send 
	  //cleanup the winsock

*/
bool initialize() {
	WSADATA DATA;
	return WSAStartup(MAKEWORD(2, 2), &DATA) == 0;
}


void interactWithClient(SOCKET clientSocket, vector<SOCKET>& clients) {
	//send/recv data to/from the client

	cout << "client connected" << endl;
	char buffer[4096]; // Buffer to hold received data

	while (1) {
		int bytesrecd = recv(clientSocket, buffer, sizeof(buffer), 0); // Receive data from the client

		if (bytesrecd <= 0) { // Check for errors or disconnection
			cout << "Client disconnected or error occurred." << endl;
			break; // Exit the loop if no data received or an error occurred
		}
		string message(buffer, bytesrecd); // Convert received data to string
		cout << "message from client:" << message << endl;

		{
			for (auto client : clients) {
				if (client != clientSocket) { // Don't send the message back to the sender
					send(client, message.c_str(), message.length(), 0); // Send the message to other clients
				}
			}
		}
	}

	auto it = find(clients.begin(), clients.end(), clientSocket); // Find the client socket in the vector
	if (it != clients.end()) {
		clients.erase(it); // Remove the client socket from the vector
	}
	closesocket(clientSocket); // Close the client socket
}

int main() {
	if (!initialize()) {
		cout << "Failed to initialize Winsock." << endl;
		return 1;
	}

	cout << "server program" << endl;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket API

	if (listenSocket == INVALID_SOCKET) {
		cout<< "Failed to create socket." << endl;
		return 1;
	}

	//create the address structure
	int port = 12345; // Port number
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET; // Address family
	serveraddr.sin_port = htons(port); // Port number and htons is an api to convert to network byte order


	//convert the ipaddress (0.0.0.0) put it inside the sin_family in binary format
	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "Setting address structure fsiled" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//bind the socket with the address structure
	if(bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "Failed to bind socket." << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//listen on the socket
	if(listen(listenSocket,SOMAXCONN)== SOCKET_ERROR) {
		cout << "Failed to listen on socket." << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "server has started listening on port :" << port << endl;

	vector<SOCKET> clients; // Vector to hold client sockets
	while (1) {
		//accept the connection from the client
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			cout << "Failed to accept connection." << endl;
		}
		clients.push_back(clientSocket); // Add the cl	ient socket to the vector
		thread t1(interactWithClient, clientSocket, std::ref(clients)); // Create a thread to handle client interaction
		t1.detach();


	}
	

	

	closesocket(listenSocket); // Close the listening socket


	WSACleanup(); // Cleanup Winsock
	return 0;
}