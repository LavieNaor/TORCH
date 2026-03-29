#include "GroupChat.h"

GroupChat::GroupChat(ChatData chatData, GroupChatData groupChatData, std::map<User, UserInChat> usersInChat, std::vector<Message> messages) : SingleChat(chatData, usersInChat, messages), m_groupChatData(groupChatData)
{
}

GroupChat::~GroupChat()
{
}

bool GroupChat::addUser(UserInChat userToAdd)
{
	this->m_usersInChat[userToAdd.user] = (userToAdd);
	return true;
}

bool GroupChat::removeUser(User userToRemove)
{
	for (auto it = this->m_usersInChat.begin(); it != this->m_usersInChat.end(); it++)
	{
		if (it->first.username == userToRemove.username)
		{
			it = this->m_usersInChat.erase(it);
			return true;
		}
	}

	return false;
}

std::string GroupChat::getGroupName()
{
	return this->m_groupChatData.groupName;
}
