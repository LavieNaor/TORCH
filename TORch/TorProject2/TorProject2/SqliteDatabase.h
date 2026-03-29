#pragma once

#include "IDatabase.h"
#include "sqlite3.h"
#include "mutex"

class SqliteDatabase : public IDatabase
{
private:

	// singleton
	static SqliteDatabase* _instance;
	static int _instanceCounter;

	// fileds
	sqlite3* _DB;

	// mutex
	std::mutex _dbLock;

	// c'tor
	SqliteDatabase();

	// d'tor
	~SqliteDatabase();

	// help functions
	void sendQuery(const char* sqlStatement, int(*callbackFunc)(void*, int, char**, char**) = nullptr, void* callbackParam = nullptr);

public:

	//sqlite singleton instance
	static SqliteDatabase* getInstance();

	//sqlite singleton delete instance
	static void deleteInstance(SqliteDatabase* instance);

	// This function opens/creates the database
	virtual bool open() override;

	// This function closes the database
	virtual bool close() override;

	// This function checks if the user exists in the DB
	virtual int doesUserExist(std::string username) override;

	// This function checks if the password matches the usernames password
	virtual int doesPasswordMatch(std::string username, std::string password) override;

	// This function adds a user
	virtual void addNewUser(std::string username, std::string password, std::string email) override;

	// This function adds a message.
	virtual void addMessage(Message message) override;

	// This function checks if the group exists in the DB
	virtual int doesGroupExist(int groupID) override;

	// This function checks if the single chat exists.
	virtual int doesSingleChatExist(User user1, User user2) override;

	// This function checks if the the user admin in chat.
	virtual int doesSomeoneAdmin(std::string username, unsigned int chatID) override;

	// this function adds a chat.
	virtual void addChat(ChatData chatData) override;

	// get all users in chat.
	virtual std::map<User, UserInChat> getAllUsersInChat(unsigned int id) override;

	// get all other users that exist.
	virtual std::vector<User> getAllOtherUsersExist(User user) override;

	// This function adds a group.
	virtual void addGroup(ChatData chatData, GroupChatData groupChatData) override;

	// This function adds a user to chat.
	virtual bool addMemberToChat(UserInChat userInChat, int groupID) override;

	// This function remove a user from group.
	virtual bool removeMemberFromGroup(User user, int groupId) override;

	// function that load chat.
	virtual std::vector<Message> loadMessagesChat(unsigned int id) override;

	// This function that return chat pointer.
	virtual SingleChat* loadChat(unsigned int id) override;

	// This function give the last chat id.
	virtual int getLastChatId() override;

	// This function returns the chat's id of the user
	virtual std::vector<unsigned int> getChatsIdsByUser(std::string username) override;

	// callback for float num
	static int callbackGetFloatNum(void* data, int argc, char** argv, char** azColName);

	// callback for int num
	static int callbackGetIntNum(void* data, int argc, char** argv, char** azColName);

	// callback for string
	static int callbackString(void* data, int argc, char** argv, char** azColName);
	
	// callback for users
	static int callbackUsers(void* data, int argc, char** argv, char** azColName);

	// callback for Messages
	static int callbackMessages(void* data, int argc, char** argv, char** azColName);

	// callback for single chat
	static int callbackChat(void* data, int argc, char** argv, char** azColName);

	// callback for group chat
	static int callbackGroupChatData(void* data, int argc, char** argv, char** azColName);

	// callback for users in chat
	static int callbackUsersInChat(void* data, int argc, char** argv, char** azColName);

	// callback for users in chat
	static int callbackChatsId(void* data, int argc, char** argv, char** azColName);
};
