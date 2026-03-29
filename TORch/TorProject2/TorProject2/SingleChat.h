#pragma once
#include <string>
#include <vector>
#include <map>
#include "Message.h"

class Message;

typedef struct ChatData
{
	unsigned int id;
	std::string chatType;
	std::time_t createdAt;
} ChatData;

class SingleChat
{
public:
	SingleChat(ChatData data, std::map<User, UserInChat> usersInChat, std::vector<Message> messages);
	~SingleChat() = default;

	// getters
	ChatData getChatData();
	std::map<User, UserInChat> getUsers();
	std::vector<Message> getAllMessages();
	std::string getType();

	// setters
	void setChatData(ChatData chatData);
	void setAllUsers(std::map<User, UserInChat> users);
	void setAllMessages(std::vector<Message> messages);


	// methods
	std::vector<User> sendMessage(Message message, User sender);
	bool isUserInChat(User userToSearch);
	void addMessage(Message message, User sender);

protected:
	// fields
	ChatData m_chatData;
	std::map<User, UserInChat> m_usersInChat;
	std::vector<Message> m_messages;

};
