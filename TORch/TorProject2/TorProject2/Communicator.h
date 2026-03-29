#pragma once
#include <iostream>
#include <stdexcept>
#include <cstring>

#include <unistd.h>         // close()
#include <arpa/inet.h>      // htons, inet_ntoa
#include <sys/socket.h>     // socket, bind, listen, accept
#include <netinet/in.h>     // sockaddr_in

#include <map>
#include <exception>
#include <string>
#include <thread>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "LoginRequestHandler.h"
#include "RequestHandlerFactory.h"

#define SERVER_PORT 8000
#define FIRST_TEN 10

typedef	int SOCKET;

class Communicator
{
public:

	//communicator singleton instance
	static Communicator* getInstance(RequestHandlerFactory* handlerFactory);

	//communicator singleton delete instance
	static void deleteInstance(Communicator* instance);

	//send
	static void send(const RequestResult result, SSL* ssl);
	
	// close socket
	static void closeSocketAndEraseSSL(SSL* ssl);

	// the method handles new clients
	void startHandleRequests();

	// static variables
	static std::map<SSL*, IRequestHandler*> m_clients;

private:

	// singleton
	static Communicator* _instance;
	static int _instanceCounter;

	// fields
	SOCKET m_serverSocket;
	SSL_CTX* m_sslCtx;
	RequestHandlerFactory* m_reqHendlerFactory;

	// c'tor
	Communicator(RequestHandlerFactory* handlerFactory);

	// d'tor
	~Communicator();

	//the method binds and listen for ports
	void bindAndListen();

	//the method handles new clients
	void handleNewClient(SOCKET sock);
};