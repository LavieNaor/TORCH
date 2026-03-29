#pragma once
#include "IDatabase.h"
#include "SingleChat.h"
#include "mutex"
#include <map>


class ChatManager
{
private:

	// singleton
	static ChatManager* _instance;
	static int _instanceCounter;

	static int _lastChatId;

	// fields
	IDatabase* m_database;
	std::map<unsigned int, SingleChat*> m_chats; // <id, chat>

	// c'tor
	ChatManager();
	ChatManager(IDatabase* DB);

	// d'tor
	~ChatManager();

	// private fuctions
	int getLastChatId();
public:

	// chatManager singleton instance
	static ChatManager* getInstance(IDatabase* DB);

	// chatManager singleton delete instance
	static void deleteInstance(ChatManager* instance);

	// methods

	const int createSingleChat(std::vector<User> users, ChatData chatData);

	const int createGroupChat(std::vector<User> users, ChatData chatData, GroupChatData groupChatData);

	// load messages of specific chat
	const std::vector<Message> loadMessagesChat(ChatData data);

	// return the id and name of all the chats the user participate in
	const std::map<unsigned int, std::string> loadChatsOfUser(std::string username);

	// return all the chats' data
	const std::vector<ChatData> getChats();

	// get the chat by the ID
	SingleChat* getChat(unsigned int id);

	// get all the user in specific chat
	std::map<User, UserInChat> getAllUsersInChat(unsigned int id);

	// get all the usernames of the others
	std::vector<User> getAllOtherUsers(User user);

	// send message - the function adds the message to the DB
	const std::vector<User> sendMessage(Message msgToSend);

	// the function add the user to group
	const bool addUser(User UserThatAdd, ChatData data, std::vector<User> usersToAdd);

	// the function remove the user from group
	const bool removeUser(User UserThatRemove, ChatData data, User userToRemove);

};