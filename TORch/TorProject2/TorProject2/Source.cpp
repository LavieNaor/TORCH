#pragma comment (lib, "ws2_32.lib")

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Server.h"
#include <iostream>
#include <exception>

int main()
{
	try
	{
		//WSAInitializer wsaInit;
		Server* myServer = Server::getInstance();
		myServer->run();
	}
	catch (std::exception& e)
	{
		std::cout << "Error occured: " << e.what() << std::endl;
	}
	
	std::cout << "Press Enter to continue...";
	std::cin.get();
	return 0;
}