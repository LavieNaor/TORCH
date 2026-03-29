#pragma once
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "LoginRequestHandler.h"
#include "LoginManager.h"
#include "ChatManager.h"
#include "MenuRequestHandler.h"
#include "IDatabase.h"
#include "SqliteDatabase.h"

class LoginRequestHandler;
class MenuRequestHandler;
class LoggedUser;

class RequestHandlerFactory
{
private:

	// singleton
	static RequestHandlerFactory* _instance;
	static int _instanceCounter;

	// managers
	LoginManager* m_loginManager;
	ChatManager* m_chatManager;

	// DB
	IDatabase* m_database;

	// c'tor
	RequestHandlerFactory(IDatabase* _database);

	// d'tor
	~RequestHandlerFactory();

public:
	
	//RHF singleton instance
	static RequestHandlerFactory* getInstance(IDatabase* _database);

	//RHF singleton delete instance
	static void deleteInstance(RequestHandlerFactory* instance);

	// The function creates a login request handler
	LoginRequestHandler* createLoginRequestHandler(SSL* clientSslSocket);

	// The function gets the login manager
	LoginManager* getLoginManager();

	MenuRequestHandler* createMenuRequestHandler(const LoggedUser user);

	ChatManager* getChatManager();

};