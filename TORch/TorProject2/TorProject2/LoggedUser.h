#pragma once
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct User
{
	std::string username;

	bool operator<(const User& other) const
	{
		return username < other.username;
	}
} User;

typedef struct UserInChat
{
	User user;
	std::string role; // MEMBER or ADMIN
	time_t joinedAt;
} UserInChat;

class LoggedUser
{
	User m_user;
	SSL* m_sslSock;
public:
	LoggedUser(const std::string username, SSL* sslSock);
	~LoggedUser();

	// The function returns the socket.
	SSL* getSslSocket() const;

	// The function returns the username.
	User getUser() const;

	// The function compares between users.
	bool operator==(const LoggedUser other) const;

	/*
	This function provides ordering for users (needed for std::map).
	input: another user.
	output: if this user is "less than" the other (always false).
	*/
	bool operator<(const LoggedUser& other) const;
};