#include "SqliteDatabase.h"
#include <map>
#include <unistd.h>
#include <cstdlib>
#include "JsonResponsePacketSerializer.h"
#include <random>
#include "ChatManager.h"
#include "Helper.h"

SqliteDatabase* SqliteDatabase::_instance = nullptr;
int SqliteDatabase::_instanceCounter = 0;

SqliteDatabase* SqliteDatabase::getInstance()
{
	SqliteDatabase::_instanceCounter++;
	if (SqliteDatabase::_instance == nullptr) //checking if we should init the db
	{
		SqliteDatabase::_instance = new SqliteDatabase();
	}

	return SqliteDatabase::_instance;
}


void SqliteDatabase::deleteInstance(SqliteDatabase* instance)
{
	if (instance != nullptr && SqliteDatabase::_instanceCounter == 1) //checking if we should delete the db
	{
		delete SqliteDatabase::_instance;
		SqliteDatabase::_instance = nullptr;
		SqliteDatabase::_instanceCounter--;
	}
	else if (instance != nullptr)
	{
		SqliteDatabase::_instanceCounter--;
	}

	instance = nullptr;
}


SqliteDatabase::SqliteDatabase() : _DB(nullptr)
{

}


SqliteDatabase::~SqliteDatabase()
{
	if (_DB)
	{
		sqlite3_close(_DB);
		_DB = nullptr;
	}
}


bool SqliteDatabase::open()
{
	std::string dbFileName = "torDB.sqlite.db";
	int file_exist = access(dbFileName.c_str(), F_OK);
	int res = sqlite3_open(dbFileName.c_str(), &_DB);
	const char* sqlStatement = nullptr;

	sqlStatement = "PRAGMA foreign_keys = ON;";
	sendQuery(sqlStatement);
	
	sqlStatement = "PRAGMA journal_mode = WAL;";
	sendQuery(sqlStatement);

	sqlStatement = "PRAGMA synchronous = NORMAL;";
	sendQuery(sqlStatement);

	if (res != SQLITE_OK)
	{
		_DB = nullptr;
		std::cout << "Failed to open DB" << std::endl;
		throw std::runtime_error("ERROR open");
	}

	if (file_exist != 0)
	{
		sqlStatement = "CREATE TABLE USERS (USERNAME TEXT PRIMARY KEY NOT NULL, PASSWORD TEXT NOT NULL, EMAIL TEXT NOT NULL);";

		sendQuery(sqlStatement);

		sqlStatement = "CREATE TABLE CHATS (ID INTEGER PRIMARY KEY NOT NULL, TYPE TEXT NOT NULL CHECK (TYPE IN ('PRIVATE', 'GROUP')), CREATED_AT TEXT NOT NULL);";

		sendQuery(sqlStatement);

		sqlStatement = "CREATE TABLE PARTICIPANT_USERS_CHATS (CHAT_ID TEXT NOT NULL, USERNAME TEXT NOT NULL, ROLE TEXT NOT NULL CHECK (ROLE IN ('MEMBER', 'ADMIN')) DEFAULT 'MEMBER', JOINED_AT TEXT NOT NULL, PRIMARY KEY(CHAT_ID, USERNAME), FOREIGN KEY (USERNAME) REFERENCES USERS(USERNAME), FOREIGN KEY (CHAT_ID) REFERENCES CHATS(ID));";

		sendQuery(sqlStatement);

		sqlStatement = "CREATE TABLE MESSAGES (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, SENDER TEXT NOT NULL, CHAT_ID INTEGER NOT NULL, CONTENT TEXT NOT NULL, TIME_SENT TEXT NOT NULL, IS_TXT_MESSAGE BOOLEAN NOT NULL, FOREIGN KEY (CHAT_ID) REFERENCES CHATS(ID), FOREIGN KEY (SENDER) REFERENCES USERS(USERNAME));";

		sendQuery(sqlStatement);

		sqlStatement = "CREATE TABLE GROUP_METADATA (CHAT_ID INTEGER PRIMARY KEY NOT NULL, NAME TEXT NOT NULL, DESCRIPTION TEXT, CREATED_BY TEXT NOT NULL, CREATED_AT INTEGER NOT NULL, FOREIGN KEY (CHAT_ID) REFERENCES CHATS(ID), FOREIGN KEY (CREATED_BY) REFERENCES USERS(USERNAME));";

		sendQuery(sqlStatement);
	}
	return true;
}

void SqliteDatabase::sendQuery(const char* sqlStatement, int(*callbackFunc)(void*, int, char**, char**), void* callbackParam)
{
	char* errMessage = nullptr;
	int res = 0;

	std::unique_lock<std::mutex> lock(this->_dbLock); // lock the DB
	res = sqlite3_exec(_DB, sqlStatement, callbackFunc, callbackParam, &errMessage);
	lock.unlock(); // unlock the DB

	if (res != SQLITE_OK)
	{
		std::cout << "" + std::string(sqlStatement) + ": " << res << std::endl;
		throw std::runtime_error("ERROR DB");
	}
}

bool SqliteDatabase::close()
{
	if (_DB)
	{
		sqlite3_close(_DB);
		_DB = nullptr;
		return true;
	}
	return false;
}

int SqliteDatabase::doesUserExist(std::string username)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	int numOfUsersWithThatUsername = 0;

	sqlStatementSTR = "SELECT COUNT(*) FROM USERS WHERE USERNAME = \"" + username + "\";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackGetIntNum, &numOfUsersWithThatUsername);

	return numOfUsersWithThatUsername;
}

int SqliteDatabase::doesSomeoneAdmin(std::string username, unsigned int chatID)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	int numOfTheUserAdminInTheChat = 0;

	sqlStatementSTR = "SELECT COUNT(*) FROM PARTICIPANT_USERS_CHATS WHERE USERNAME = \"" + username + "\" AND CHAT_ID = " + std::to_string(chatID) + " AND ROLE = \"ADMIN\";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackGetIntNum, &numOfTheUserAdminInTheChat);

	return numOfTheUserAdminInTheChat;
}

int SqliteDatabase::doesPasswordMatch(std::string username, std::string password)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	int numOfUsersWithThatUsernameAndThatPassword = 0;

	sqlStatementSTR = "SELECT COUNT(*) FROM USERS WHERE USERNAME = \"" + username + "\" AND PASSWORD = \"" + password + "\";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackGetIntNum, &numOfUsersWithThatUsernameAndThatPassword);

	return numOfUsersWithThatUsernameAndThatPassword;
}

void SqliteDatabase::addNewUser(std::string username, std::string password, std::string email)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";

	sqlStatementSTR = "INSERT INTO USERS (USERNAME, PASSWORD, EMAIL) VALUES(\"" + username + "\", \"" + password + "\", \"" + email + "\");";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement);
}

void SqliteDatabase::addMessage(Message message)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";

	sqlStatementSTR = "INSERT INTO MESSAGES (SENDER, CHAT_ID, CONTENT, TIME_SENT, IS_TXT_MESSAGE) VALUES(\"" + message.sender.username + "\", " + std::to_string(message.chatId) + ", \"" + message.content + "\", \"" + Helper::toIso8601UTC(message.time) + "\", " + std::to_string(1) + ");";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement);
}

void SqliteDatabase::addGroup(ChatData chatData, GroupChatData groupChatData)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";

	sqlStatement = "BEGIN TRANSACTION;";
	sendQuery(sqlStatement);

	addChat(chatData);

	sqlStatementSTR = "INSERT INTO GROUP_METADATA (CHAT_ID, NAME, DESCRIPTION, CREATED_BY, CREATED_AT) VALUES(" + std::to_string(chatData.id) + ", \"" + groupChatData.groupName + "\", \"" + groupChatData.description + "\", \"" + groupChatData.createdBy.username + "\", \"" + Helper::toIso8601UTC(chatData.createdAt) + "\"); ";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement);

	sqlStatement = "END TRANSACTION;";
	sendQuery(sqlStatement);
}

int SqliteDatabase::doesGroupExist(int groupID)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	int numOfGroupsWithThatName = 0;

	sqlStatementSTR = "SELECT COUNT(*) FROM GROUPS WHERE ID = \"" + std::to_string(groupID) + "\";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackGetIntNum, &numOfGroupsWithThatName);

	return numOfGroupsWithThatName;
}

int SqliteDatabase::doesSingleChatExist(User user1, User user2)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	int numOSingleChatsOfBothUsersTogether = 0;

	sqlStatementSTR = "SELECT COUNT(*) FROM PARTICIPANT_USERS_CHATS P1 INNER JOIN PARTICIPANT_USERS_CHATS P2 ON P1.CHAT_ID = P2.CHAT_ID INNER JOIN CHATS C ON C.ID = P1.CHAT_ID WHERE P1.USERNAME = \"" + user1.username + "\" AND P2.USERNAME = \"" + user2.username + "\" AND C.TYPE = 'PRIVATE';;";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackGetIntNum, &numOSingleChatsOfBothUsersTogether);

	return numOSingleChatsOfBothUsersTogether;
}

void SqliteDatabase::addChat(ChatData chatData)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";

	sqlStatementSTR = "INSERT INTO CHATS (ID, TYPE, CREATED_AT) VALUES(" + std::to_string(chatData.id) + ", \"" + chatData.chatType + "\", \"" + Helper::toIso8601UTC(chatData.createdAt) + "\");";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement);
}

std::map<User, UserInChat> SqliteDatabase::getAllUsersInChat(unsigned int id)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	std::map<User, UserInChat> usersInChat;

	sqlStatementSTR = "SELECT USERNAME FROM PARTICIPANT_USERS_CHATS WHERE CHAT_ID = " + std::to_string(id) + ";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackUsersInChat, &usersInChat);

	return usersInChat;
}

std::vector<User> SqliteDatabase::getAllOtherUsersExist(User user)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	std::vector<User> allUsers;

	sqlStatementSTR = "SELECT USERNAME FROM USERS WHERE USERNAME != \"" + user.username + "\";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackUsers, &allUsers);

	return allUsers;
}

bool SqliteDatabase::addMemberToChat(UserInChat userInChat, int groupId)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";

	sqlStatementSTR = "INSERT INTO PARTICIPANT_USERS_CHATS (CHAT_ID, USERNAME, ROLE, JOINED_AT) VALUES(" + std::to_string(groupId) + ", \"" + userInChat.user.username + "\", \"" + userInChat.role + "\", \"" + Helper::toIso8601UTC(userInChat.joinedAt) + "\");";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement);

	return true;
}

bool SqliteDatabase::removeMemberFromGroup(User user, int groupId)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";

	sqlStatementSTR = "DELETE FROM PARTICIPANT_USERS_CHATS WHERE USERNAME = \"" + user.username + "\" AND CHAT_ID = " + std::to_string(groupId) + "";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement);

	return true;
}

std::vector<Message> SqliteDatabase::loadMessagesChat(unsigned int id)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	std::vector<Message> allMessages;

	sqlStatementSTR = "SELECT * FROM MESSAGES WHERE CHAT_ID = " + std::to_string(id) + ";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackMessages, &allMessages);

	return allMessages;
}

SingleChat* SqliteDatabase::loadChat(unsigned int id)
{
	const char* sqlStatement = nullptr;
	std::string sqlStatementSTR = "";
	std::vector<Message> allMessages;
	std::map<User, UserInChat> usersInChat;
	ChatData chatData;
	GroupChatData groupChatData;
	SingleChat* chat = nullptr;

	sqlStatementSTR = "SELECT * FROM CHATS WHERE ID = " + std::to_string(id) + ";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackChat, &chatData);

	sqlStatementSTR = "SELECT * FROM MESSAGES WHERE CHAT_ID = " + std::to_string(id) + ";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackMessages, &allMessages);

	sqlStatementSTR = "SELECT * FROM PARTICIPANT_USERS_CHATS WHERE CHAT_ID = " + std::to_string(id) + ";";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackUsersInChat, &usersInChat); // participant instead of users -> in the feature

	if (chatData.chatType.compare("GROUP") == 0)
	{
		sqlStatementSTR = "SELECT * FROM GROUP_METADATA WHERE CHAT_ID = " + std::to_string(id) + ";";
		sqlStatement = sqlStatementSTR.c_str();

		sendQuery(sqlStatement, callbackGroupChatData, &groupChatData);

		chat = new GroupChat(chatData, groupChatData, usersInChat, allMessages);
	}
	else
	{
		chat = new SingleChat(chatData, usersInChat, allMessages);
	}

	return chat;
}

int SqliteDatabase::getLastChatId()
{
	const char* sqlStatement = nullptr;
	int lastId = 0;

	sqlStatement = "SELECT IFNULL(MAX(ID), 0) FROM CHATS;";

	sendQuery(sqlStatement, callbackGetIntNum, &lastId);

	return lastId;
}

std::vector<unsigned int> SqliteDatabase::getChatsIdsByUser(std::string username)
{
	const char* sqlStatement = nullptr;
	std::vector<unsigned int> chatsId;
	std::string sqlStatementSTR = "";

	sqlStatementSTR = "SELECT CHAT_ID FROM PARTICIPANT_USERS_CHATS WHERE USERNAME = \"" + username + "\"";
	sqlStatement = sqlStatementSTR.c_str();

	sendQuery(sqlStatement, callbackChatsId, &chatsId);

	return chatsId;
}

int SqliteDatabase::callbackGetFloatNum(void* data, int argc, char** argv, char** azColName)
{
	float* num = (float*)data;
	std::string str = "";

	if (argc > 0)
	{
		try
		{
			str = argv[0];
			*num = std::stod(str);
		}
		catch (const std::invalid_argument& e)
		{
			std::cerr << "invalid string!!!" << std::endl;
		}
		catch (const std::out_of_range& e)
		{
			std::cerr << "out of range double!!!" << std::endl;
		}
	}

	return 0;
}

int SqliteDatabase::callbackGetIntNum(void* data, int argc, char** argv, char** azColName)
{
	int* num = (int*)data;

	if (argc > 0)
	{
		*num = atoi(argv[0]);
	}
	return 0;
}

int SqliteDatabase::callbackString(void* data, int argc, char** argv, char** azColName)
{
	std::string* str = (std::string*)data;

	if (argc > 0)
	{
		*str = atoi(argv[0]);
	}
	return 0;
}

int SqliteDatabase::callbackUsers(void* data, int argc, char** argv, char** azColName)
{
	std::vector<User>* users = (std::vector<User>*)data;
	User user;
	user.username = "";
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "USERNAME")
		{
			user.username = argv[i];
		}
	}

	(*users).push_back(user);

	return 0;
}

int SqliteDatabase::callbackUsersInChat(void* data, int argc, char** argv, char** azColName)
{
	std::map<User, UserInChat>* usersInChat = (std::map<User, UserInChat>*)data;
	UserInChat userInChat;
	userInChat.user.username = "";
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "USERNAME")
		{
			userInChat.user.username = argv[i];
		}
		else if (std::string(azColName[i]) == "ROLE")
		{
			userInChat.role = argv[i];
		}
		else if (std::string(azColName[i]) == "JOINED_AT")
		{
			userInChat.joinedAt = Helper::fromIso8601UTC(argv[i]);
		}
	}

	(*usersInChat)[userInChat.user] = userInChat;

	return 0;
}

int SqliteDatabase::callbackChatsId(void* data, int argc, char** argv, char** azColName)
{
	std::vector<int>* chatsId = (std::vector<int>*)data;

	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "CHAT_ID")
		{
			chatsId->push_back(std::stoi(argv[i]));
		}
	}
	return 0;
}

int SqliteDatabase::callbackMessages(void* data, int argc, char** argv, char** azColName)
{
	std::vector<Message>* messages = (std::vector<Message>*)data;
	Message message;

	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "SENDER")
		{
			message.sender.username = argv[i];
		}
		else if (std::string(azColName[i]) == "CHAT_ID")
		{
			message.chatId = std::stoi(argv[i]);
		}
		else if (std::string(azColName[i]) == "CONTENT")
		{
			message.content = argv[i];
		}
		else if (std::string(azColName[i]) == "TIME_SENT")
		{
			message.time = Helper::fromIso8601UTC(argv[i]);
		}
		else if (std::string(azColName[i]) == "IS_TXT_MESSAGE")
		{
			message.isTxtMsg = std::stoi(argv[i]);
		}
	}

	(*messages).push_back(message);

	return 0;
}

int SqliteDatabase::callbackChat(void* data, int argc, char** argv, char** azColName)
{
	ChatData* chatData = (ChatData*)data;

	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "ID")
		{
			(*chatData).id = std::atoi(argv[i]);
		}
		else if (std::string(azColName[i]) == "TYPE")
		{
			(*chatData).chatType = argv[i];
		}
		else if (std::string(azColName[i]) == "CREATED_AT")
		{
			(*chatData).createdAt = Helper::fromIso8601UTC(argv[i]);
		}
	}

	return 0;
}

int SqliteDatabase::callbackGroupChatData(void* data, int argc, char** argv, char** azColName)
{
	GroupChatData* groupChatData = (GroupChatData*)data;

	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "CREATED_BY")
		{
			(*groupChatData).createdBy.username = argv[i];
		}
		else if (std::string(azColName[i]) == "DESCREPTION")
		{
			(*groupChatData).description = argv[i];
		}
		else if (std::string(azColName[i]) == "NAME")
		{
			(*groupChatData).groupName = argv[i];
		}
	}

	return 0;
}
