#pragma once
#include "IDatabase.h"
#include "LoggedUser.h"
#include "SqliteDatabase.h"
#include <vector>
#include <mutex>

class LoginManager
{
private:

	// singleton
	static LoginManager* _instance;
	static int _instanceCounter;

	// fields
	IDatabase* m_database;
	std::vector<LoggedUser> m_loggedUsers;

	// mutex
	std::mutex _lockDB;
	std::mutex _lockLog; // logged users vector

	// c'tor
	LoginManager();
	LoginManager(IDatabase* DB);

	// d'tor
	~LoginManager();

public:

	//loginManager singleton instance
	static LoginManager* getInstance(IDatabase* DB);

	//loginManager singleton delete instance
	static void deleteInstance(LoginManager* instance);

	// signup method
	const bool signup(const std::string username, const std::string password, const std::string email, const LoggedUser newUserLoggedIn);

	// login method
	const bool login(const std::string username, const std::string password, const LoggedUser newUserLoggedIn);

	// logout method
	void logout(const std::string username);

	std::vector<LoggedUser> getLoggedUsersToSend(const std::vector<User> usersToSend);
};