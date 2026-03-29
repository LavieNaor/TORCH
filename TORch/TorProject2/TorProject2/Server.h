#pragma once
#include "Communicator.h"
#include "IDatabase.h"
#include "RequestHandlerFactory.h"

class IDataBase;

class RequestHandlerFactory;

class Server
{
public:

	//server singleton instance
	static Server* getInstance();

	//server singleton delete instance
	static void deleteInstance(Server* instance);

	//This function starts to run the server.
	void run();

private:

	// c'tor
	Server();

	// d'tor
	~Server();

	// fields
	IDatabase* m_database;
	RequestHandlerFactory* m_hendlerFactory;
	Communicator* m_communicator;

	// singleton
	static Server* _instance;
	static int _instanceCounter;
};