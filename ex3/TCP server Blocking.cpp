#include <iostream>
using namespace std;
// Don't forget to include "Ws2_32.lib" in the library list.
#include <winsock2.h>
#include <string.h>
#include <time.h>

const int TIME_PORT = 27015;

void main() 
{
    // Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
    // The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData; 
    	
	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2,2), &wsaData))
	{
        cout<<"Time Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.
	// Listen through the socket for incoming connections.

    // After initialization, a SOCKET object is ready to be instantiated.
	
	// Create a SOCKET object called listenSocket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							streaming sockets (SOCK_STREAM), 
	//							and the TCP/IP protocol (IPPROTO_TCP).
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The if statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == listenSocket)
	{
        cout<<"Time Server: Error at socket(): "<<WSAGetLastError()<<endl;
        WSACleanup();
        return;
	}

	// For a server to communicate on a network, it must bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

    // Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
    serverService.sin_family = AF_INET; 
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.

    // The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
    if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR *) &serverService, sizeof(serverService))) 
	{
		cout<<"Time Server: Error at bind(): "<<WSAGetLastError()<<endl;
        closesocket(listenSocket);
		WSACleanup();
        return;
    }

    // Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
    if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
        closesocket(listenSocket);
		WSACleanup();
        return;
	}

    // Accept connections and handles them one by one.
	while (true)
	{
		struct sockaddr_in from;		// Address of sending partner
		int fromLen = sizeof(from);

		cout<<"Time Server: Wait for clients' requests.\n";	
		
		// The accept function permits an incoming connection
		// attempt on another socket (msgSocket).
		// The first argument is a bounded-listening socket that 
		// receives connections.
		// The second argument is an optional pointer to buffer 
		// that receives the internet address of the connecting enrity.
		// the third one is a pointer to the length of the network address
		// (given in the second argument). 
		SOCKET msgSocket = accept(listenSocket, (struct sockaddr *)&from, &fromLen);
		
		if (INVALID_SOCKET == msgSocket)
		{ 
			 cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl; 
			 closesocket(listenSocket);	 
			 WSACleanup();
			 return;
		}
	
		cout << "Time Server: Client "<<inet_ntoa(from.sin_addr)<<":"<<ntohs(from.sin_port)<<" is connected." << endl;

		// Send and receive data.
		int bytesSent = 0;
		int bytesRecv = 0;
		char sendBuff[255];
		char recvBuff[255];
	    
		// Get client's requests and answer them.
		// The recv function receives data from a connected or bound socket.
		// The buffer for data to be received and its available size are 
		// returned by recv. The last argument is an idicator specifying the way 
		// in which the call is made (0 for default).
		bytesRecv = recv(msgSocket, recvBuff, 255, 0);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
			closesocket(listenSocket);	
			closesocket(msgSocket);			
			WSACleanup();
			return;
		}
		if (bytesRecv == 0)
			cout<<"Client closed the connection\n";
		else
		{
			recvBuff[bytesRecv]='\0'; //add the null-terminating to make it a string
			cout<<"Time Server: Recieved: "<<bytesRecv<<" bytes of \""<<recvBuff<<"\" message.\n";

			if (strncmp(recvBuff, "TimeString", 10) == 0)
			{
				// Answer client's request by the current time string.
				
				// Get the current time.
				time_t timer;
				time(&timer);
				// Parse the current time to printable string.
				strcpy(sendBuff, ctime(&timer));
				sendBuff[strlen(sendBuff)-1] = 0; //to remove the new-line from the created string	
			}
			else if (strncmp(recvBuff, "SecondsSince1970", 16) == 0)
			{
				// Answer client's request by the current time in seconds.
				
				// Get the current time.
				time_t timer;
				time(&timer);
				// Convert the number to string.
				itoa((int)timer, sendBuff, 10);		
			}
			
			//If it is not Exit, send the required data
			//Otherwise, do nothing except for closing the connection
			if (strncmp(recvBuff, "Exit", 4) != 0)
			{
				bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
				if (SOCKET_ERROR == bytesSent)
				{
					cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
					closesocket(listenSocket);	
					closesocket(msgSocket);				
					WSACleanup();
					return;
				}
				cout<<"Time Server: Sent: "<<bytesSent<<"\\"<<strlen(sendBuff)<<" bytes of \""<<sendBuff<<"\" message.\n";	
			}
		}
		closesocket(msgSocket);
	}

	// Closing connections and Winsock.
	cout<<"Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}