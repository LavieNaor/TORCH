#include "Server.h"

Server* Server::_instance = nullptr;
int Server::_instanceCounter = 0;

Server* Server::getInstance()
{
	Server::_instanceCounter++; //adding to instance

	if (Server::_instance == nullptr) //checking if we should init the server
	{
		Server::_instance = new Server();
	}

	return Server::_instance;
}

void Server::deleteInstance(Server* instance)
{
	if (instance != nullptr && Server::_instanceCounter == 1)//checking if we need to delete the server
	{
		delete Server::_instance;
		Server::_instance = nullptr;
		Server::_instanceCounter--;
	}
	else if (instance != nullptr)//deleting the server
	{
		Server::_instanceCounter--;
	}

	instance = nullptr;
}

Server::Server()
{
	this->m_database = SqliteDatabase::getInstance();
	((SqliteDatabase*)this->m_database)->open();
	this->m_hendlerFactory = RequestHandlerFactory::getInstance(this->m_database);
	this->m_communicator = Communicator::getInstance(m_hendlerFactory);
}


Server::~Server()
{
	((SqliteDatabase*)this->m_database)->close();
	Communicator::deleteInstance(this->m_communicator);
	RequestHandlerFactory::deleteInstance(this->m_hendlerFactory);
}


void Server::run()
{
	std::string input = "";
	std::thread t_connector(&Communicator::startHandleRequests, this->m_communicator);
	t_connector.detach();
	while (input.compare("EXIT"))
	{
		//std::cout << "Enter Console Input: " << std::endl;
		//std::cin >> input;
	}
}