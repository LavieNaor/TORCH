#include "RequestHandlerFactory.h"

RequestHandlerFactory* RequestHandlerFactory::_instance = nullptr;
int RequestHandlerFactory::_instanceCounter = 0;

RequestHandlerFactory* RequestHandlerFactory::getInstance(IDatabase* _database)
{
	RequestHandlerFactory::_instanceCounter++;//adding one to instance counter
	if (RequestHandlerFactory::_instance == nullptr)//checking if we should init the request handler factory
	{
		RequestHandlerFactory::_instance = new RequestHandlerFactory(_database);
	}

	return RequestHandlerFactory::_instance;
}

void RequestHandlerFactory::deleteInstance(RequestHandlerFactory* instance)
{
	if (RequestHandlerFactory::_instance != nullptr && RequestHandlerFactory::_instanceCounter == 1)//checking if we should delete the instance
	{
		delete RequestHandlerFactory::_instance;//deleting instance
		RequestHandlerFactory::_instance = nullptr;
		RequestHandlerFactory::_instanceCounter--;//removing one from counter
	}
	else if (RequestHandlerFactory::_instance != nullptr)
	{
		RequestHandlerFactory::_instanceCounter--;//removing one from counter
	}

	instance = nullptr;
}


RequestHandlerFactory::RequestHandlerFactory(IDatabase* _database) : m_database(_database), m_loginManager(LoginManager::getInstance(_database)), m_chatManager(ChatManager::getInstance(_database))
{
}

RequestHandlerFactory::~RequestHandlerFactory()
{
	this->m_database = nullptr;
	LoginManager::deleteInstance(this->m_loginManager);
}


LoginRequestHandler* RequestHandlerFactory::createLoginRequestHandler(SSL* clientSslSocket)
{
	return new LoginRequestHandler(this, clientSslSocket);
}

LoginManager* RequestHandlerFactory::getLoginManager()
{
	return m_loginManager;
}

MenuRequestHandler* RequestHandlerFactory::createMenuRequestHandler(const LoggedUser user)
{
	return new MenuRequestHandler(this, user);
}

ChatManager* RequestHandlerFactory::getChatManager()
{
	return m_chatManager;
}
