#include "Communicator.h"
#include "JsonRequestPacketDeserializer.h"
#include "JsonResponsePacketSerializer.h" 
#include <sstream>
#include <iomanip>

Communicator* Communicator::_instance = nullptr;
int Communicator::_instanceCounter = 0;
std::map<SSL*, IRequestHandler*> Communicator::m_clients;

Communicator* Communicator::getInstance(RequestHandlerFactory* handlerFactory)
{
	Communicator::_instanceCounter++; //adding one instance to signleton counter

	if (Communicator::_instance == nullptr) //checking if we should init the communicator
	{
		Communicator::_instance = new Communicator(handlerFactory);
	}

	return Communicator::_instance; //returns new instance as nullptr
}


void Communicator::deleteInstance(Communicator* instance)
{
	if (instance != nullptr && Communicator::_instanceCounter == 1) //checking if we should delete the communicator
	{
		delete Communicator::_instance;
		Communicator::_instance = nullptr;
		Communicator::_instanceCounter--;
	}
	else if (instance != nullptr)
	{
		Communicator::_instanceCounter--; //removing one instance of signleton
	}

	instance = nullptr; //returntin new instance as nullptr
}

void Communicator::send(const RequestResult result, SSL* ssl)
{
	std::string ans = "";
	int lengthOfBitsMsg = 0;
	std::ostringstream oss;

	try
	{
		ans = std::string(result.response.begin(), result.response.end());

		oss << std::setfill('0') << std::setw(10) << ans.length();

		ans = oss.str() + ans;

		SSL_write(ssl, ans.c_str(), ans.length());
	}
	catch (const std::exception& e)
	{
		Communicator::closeSocketAndEraseSSL(ssl);
	}
}

void Communicator::closeSocketAndEraseSSL(SSL* ssl)
{
	SOCKET socketToClose = SSL_get_fd(ssl);

	m_clients.erase(ssl);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	close(socketToClose);
	ssl = nullptr;
}


Communicator::Communicator(RequestHandlerFactory* handlerFactory): m_reqHendlerFactory(handlerFactory)
{
	// this server use TCP. that why SOCK_STREAM & IPPROTO_TCP
	// if the server use UDP we will use: SOCK_DGRAM & IPPROTO_UDP
	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_serverSocket < 0)
		throw std::runtime_error(std::string(__FUNCTION__) + " - socket");

	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	const SSL_METHOD* method = TLS_server_method();
	m_sslCtx = SSL_CTX_new(method);
	if (!m_sslCtx)
		throw std::runtime_error("Unable to create SSL context");

	if (SSL_CTX_use_certificate_file(m_sslCtx, "server.crt", SSL_FILETYPE_PEM) <= 0)
		throw std::runtime_error("Failed to load certificate");

	if (SSL_CTX_use_PrivateKey_file(m_sslCtx, "server.key", SSL_FILETYPE_PEM) <= 0)
		throw std::runtime_error("Failed to load private key");

	SSL_CTX_set_verify(m_sslCtx, SSL_VERIFY_NONE, nullptr); // one side TLS
}

Communicator::~Communicator()
{
	std::cout << __FUNCTION__ << " closing accepting socket" << std::endl;
	
	try
	{
		for (auto it = this->m_clients.begin(); it != this->m_clients.end(); it++)
		{
			//closing all client socket on the sockets map

			//deleting the handler attached to the socket
			delete it->second;

			Communicator::closeSocketAndEraseSSL(it->first);
		}
		close(m_serverSocket);//closing server socket
		SSL_CTX_free(m_sslCtx);
	}
	catch (...) {}
}


void Communicator::startHandleRequests()
{
	bindAndListen();

	try
	{
		while (true)
		{
			// the main thread is only accepting clients 
			// and add then to the list of handlers
			std::cout << "Waiting for client connection request" << std::endl;

			// this accepts the client and create a specific socket from server to this client
			// the process will not continue until a client connects to the server
			SOCKET client_socket = accept(m_serverSocket, NULL, NULL);
			if (client_socket == -1)
				throw std::runtime_error(std::string(__FUNCTION__));

			std::cout << "Client accepted. Server and client can speak" << std::endl;

			// the function that handle the conversation with the client
			std::thread T(&Communicator::handleNewClient, this, client_socket);
			T.detach();
		}
	}
	catch (std::exception& e)
	{

	}
}

void Communicator::bindAndListen()
{
	struct sockaddr_in sa = { 0 };
	sa.sin_port = htons(SERVER_PORT);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;

	// again stepping out to the global namespace
	if (::bind(m_serverSocket, (struct sockaddr*)&sa, sizeof(sa)) < 0)
		throw std::runtime_error(std::string(__FUNCTION__) + " - bind");
	std::cout << "binded" << std::endl;

	if (::listen(m_serverSocket, 5) < 0)
		throw std::runtime_error(std::string(__FUNCTION__) + " - listen");
	std::cout << "listening..." << std::endl;
}

void Communicator::handleNewClient(SOCKET client_socket)
{
	int err = 0;
	bool succeded = false;
	std::string lengthMsg = "";
	std::string msg = "";
	LoginRequestHandler* handle;
	RequestResult result;
	RequestInfo inf;
	std::string ans = "";
	std::string msgData = "";
	ErrorResponse respError;
	std::string id = "";
	int lengthOfMsg = 0;
	std::ostringstream oss;
	char firstTenBytes[FIRST_TEN];
	char* charMsg = nullptr;
	SSL* ssl = nullptr;

	ssl = SSL_new(m_sslCtx);

	if (!ssl) 
	{
		ERR_print_errors_fp(stderr);
		close(client_socket);
		return;
	}

	SSL_set_fd(ssl, client_socket);

	if (SSL_accept(ssl) <= 0) {
		ERR_print_errors_fp(stderr);
		close(client_socket);
		SSL_free(ssl);
		return;
	}

	try
	{
		this->m_clients[ssl] = m_reqHendlerFactory->createLoginRequestHandler(ssl);
		while (true)
		{
			err = SSL_read(ssl, firstTenBytes, FIRST_TEN);
			if (err == 0 || err == -1)
			{
				throw std::runtime_error("client disconnected");
			}
			msg = firstTenBytes;

			if (msg.length() < FIRST_TEN)
			{
				ans = "{message: \"Less Then 6 Characters.\"}";
				SSL_write(ssl, ans.c_str(), ans.length());
			}
			else
			{
				lengthMsg += firstTenBytes[0];
				lengthMsg += firstTenBytes[1];
				lengthMsg += firstTenBytes[2];
				lengthMsg += firstTenBytes[3];
				lengthMsg += firstTenBytes[4];
				lengthMsg += firstTenBytes[5];
				lengthMsg += firstTenBytes[6];
				lengthMsg += firstTenBytes[7];
				lengthMsg += firstTenBytes[8];
				lengthMsg += firstTenBytes[9];

				int length = std::stoi(lengthMsg);

				charMsg = new char[length + 1];
				err = SSL_read(ssl, charMsg, length);
				if (err == 0 || err == -1)
				{
					throw std::runtime_error("client disconnected");
				}
				inf.id = (int)charMsg[0] - 48;
				charMsg[length] = '\0';
				time(&inf.receivalTime);
				inf.receivalTime += 2 * 60 * 60;
				msgData = charMsg;
				std::cout << msgData << std::endl;
				inf.buffer = {};
				for (int i = 1; i < msgData.length(); i++)
				{
					inf.buffer.push_back((unsigned char)msgData[i]);
				}
				result = this->m_clients[ssl]->handleRequest(inf);
				
				Communicator::send(result, ssl);

				lengthMsg = "";
				oss.str("");
				oss.clear();

				if (result.newHandler != nullptr)
				{
					this->m_clients.erase(ssl);
					this->m_clients.insert({ ssl, result.newHandler });
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		RequestInfo inf;
		inf.id = SIGNOUT_CODE; // change to code of the hendler after login
		if (this->m_clients[ssl]->isRequestRelevant(inf))
		{
			this->m_clients[ssl]->handleRequest(inf);
		}
		Communicator::closeSocketAndEraseSSL(ssl);

		if (charMsg)
		{
			delete[] charMsg;
			charMsg = nullptr;
		}
	}
}