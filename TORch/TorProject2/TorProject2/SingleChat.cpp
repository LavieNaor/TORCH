#include "SingleChat.h"

SingleChat::SingleChat(ChatData data, std::map<User, UserInChat> usersInChat, std::vector<Message> messages): m_chatData(data), m_usersInChat(usersInChat), m_messages(messages)
{
}


ChatData SingleChat::getChatData()
{
	return this->m_chatData;
}


std::map<User, UserInChat> SingleChat::getUsers()
{
	return this->m_usersInChat;
}


std::vector<Message> SingleChat::getAllMessages()
{
	return this->m_messages;
}

std::string SingleChat::getType()
{
	return this->m_chatData.chatType;
}


void SingleChat::setChatData(ChatData chatData)
{
	this->m_chatData = chatData;
}


void SingleChat::setAllUsers(std::map<User, UserInChat> users)
{
	this->m_usersInChat = users;
}


void SingleChat::setAllMessages(std::vector<Message> messages)
{
	this->m_messages = messages;
}


std::vector<User> SingleChat::sendMessage(Message message, User sender)
{	
	// add the message
	addMessage(message, sender);

	// return vector of the recivers users
	std::vector<User> recivers;
	for (auto it = this->m_usersInChat.begin(); it != this->m_usersInChat.end(); it++)
	{
		if (it->first.username != sender.username)
		{
			recivers.push_back(it->first);
		}
	}

	return recivers;
}


bool SingleChat::isUserInChat(User userToSearch)
{
	for (auto it = m_usersInChat.begin(); it != m_usersInChat.end(); it++)
	{
		if (it->first.username == userToSearch.username)
		{
			return true;
		}
	}
	return false;
}


void SingleChat::addMessage(Message message, User sender)
{
	// push the message to the messages vector
	this->m_messages.push_back(message);
}






