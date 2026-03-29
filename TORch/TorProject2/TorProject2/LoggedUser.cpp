#include "LoggedUser.h"

LoggedUser::LoggedUser(std::string username, SSL* sslSock) : m_sslSock(sslSock)
{
	m_user.username = username;
}

LoggedUser::~LoggedUser()
{
}

SSL* LoggedUser::getSslSocket() const
{
	return m_sslSock;
}

User LoggedUser::getUser() const
{
	return m_user;
}

bool LoggedUser::operator==(const LoggedUser other) const
{
	return m_user.username == other.getUser().username && m_sslSock == other.getSslSocket();
}

bool LoggedUser::operator<(const LoggedUser& other) const
{
	return m_user.username < other.getUser().username && m_sslSock < other.getSslSocket();
}