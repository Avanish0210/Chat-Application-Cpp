#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>

using namespace std;
#pragma comment(lib, "ws2_32.lib") // Winsock library

/*
	initialize winsock
	create socket
	conect to server 
	send/receive data
	close socket

*/

bool initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}


void sendMsg(SOCKET s) {
	cout << "enter your chat name :" << endl;
	string name;
	getline(cin, name);
	string message;

    // No other changes are needed in the existing code.
	while (1) {
		getline(cin, message);
		string msg = name + ": " + message; // Prepend name to the message
		int bytesent = send(s, msg.c_str(), msg.length(), 0);
		if(bytesent == SOCKET_ERROR) {
			cout << "Failed to send message: " << endl;
			break; // Exit the loop if sending fails
		}

		if(message == "quit") { // Check for exit command
			cout << "stopping the application" << endl;
			break; // Exit the loop if user types "exit"
		}

	}
	closesocket(s); // Close the socket when done
	WSACleanup(); // Cleanup Winsock

}

void ReceiveMsg(SOCKET s) {
	char buffer[4090]; // Buffer for receiving messages
	int recvlength;
	string msg = ""; // String to hold the received message
	while (1) {
		recvlength = recv(s, buffer, sizeof(buffer), 0); // Receive data
		if(recvlength <= 0) {
			cout << "Failed to receive message or connection closed: " << endl;
			break; // Exit the loop if receiving fails or connection is closed
		}
		else {
			msg = string(buffer, recvlength); // Convert received data to string
			cout << msg << endl; // Print the received message
		}
	}
	closesocket(s); // Close the socket when done
	WSACleanup();

}


int main() {

	if(!initialize()) {
		cerr << "Failed to initialize Winsock" << endl;
		return 1;
	}

	SOCKET s;  // Socket descriptor
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		cout << "Failed to create socket: " << endl;
		return 1;
	}
	int port = 12345;
	string serveraddress = "127.0.0.1"; // Localhost
	sockaddr_in serveraddr; // Server address structure
	serveraddr.sin_family = AF_INET; // Address family
	serveraddr.sin_port = htons(port); // Port number 
	inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr)); // Convert IP address from string to binary form


	if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "not able to connect to server: " << endl;
		closesocket(s); // Close the socket if connection fails
		WSACleanup(); // Cleanup Winsock
		return 1;
	}

	cout << "connected to server" << endl;

	thread senderthread(sendMsg, s); // Create a thread to send messages
	thread receiver(ReceiveMsg, s);

	senderthread.join(); 
	receiver.join(); 
	return 0;
}		