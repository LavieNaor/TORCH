#pragma once
#include "SingleChat.h"

typedef struct GroupChatData
{
	std::string groupName;
	std::string description;
	User createdBy;
} GroupChatData;

class GroupChat : public SingleChat
{
private:
	GroupChatData m_groupChatData;

public:

	GroupChat(ChatData chatData, GroupChatData groupChatData, std::map<User, UserInChat> usersInChat, std::vector<Message> messages); // c'tor
	~GroupChat(); // d'tor

	// methods
	bool addUser(UserInChat userToAdd);
	bool removeUser(User userToRemove);

	// getters
	std::string getGroupName();
};