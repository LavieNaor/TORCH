#pragma once

#include <iostream>
#include <string>
#include <list>
#include "GroupChat.h"
#include "Message.h"

class LoggedUser;

class IDatabase
{
public:
	virtual ~IDatabase() = default;

	/*
	This function opens/creates the database.
	input: none.
	output: if it worked.
	*/
	virtual bool open() = 0;
	/*
	This function closes the database.
	input: none.
	output: if it was open.
	*/
	virtual bool close() = 0;

	/*
	This function checks if the user exists.
	input: a username.
	output: number of users with that username.
	*/
	virtual int doesUserExist(std::string username) = 0;
	/*
	This function checks if the password matches the usernames password.
	input: a username and a password.
	output: how many match.
	*/
	virtual int doesPasswordMatch(std::string username, std::string password) = 0;

	/*
	This function adds a user.
	input: a username, password and email.
	output: none.
	*/
	virtual void addNewUser(std::string username, std::string password, std::string email) = 0;

	/*
	This function adds a message.
	input: a username, password and email.
	output: none.
	*/
	virtual void addMessage(Message message) = 0;
	
	/*
	This function checks if the group exists.
	input: a groupname.
	output: number of groups with that groupname.
	*/
	virtual int doesGroupExist(int groupID) = 0;

	/*
	This function checks if the single chat exists.
	input: two users.
	output: 1 if the single chat exists, otherwise 0.
	*/
	virtual int doesSingleChatExist(User user1, User user2) = 0;

	/*
	This function checks if the the user admin in chat.
	input: a chat id, username.
	output: 1 the the user admin in chat, otherwise 0.
	*/
	virtual int doesSomeoneAdmin(std::string username, unsigned int chatID) = 0;

	/*
	This function adds a chat.
	input: a chat data.
	output: none.
	*/
	virtual void addChat(ChatData chatData) = 0;

	/*
	This function that return vector of all users in the chat.
	input: a chat id.
	output: vector of all users in the chat.
	*/
	virtual std::map<User, UserInChat> getAllUsersInChat(unsigned int id) = 0;

	/*
	This function that return vector of all other users that exist.
	input: none.
	output: vector of all users that exist.
	*/
	virtual std::vector<User> getAllOtherUsersExist(User user) = 0;

	/*
	This function that return vector of all messages in chat.
	input: a chat id.
	output: vector of all messages in chat.
	*/
	virtual std::vector<Message> loadMessagesChat(unsigned int id) = 0;

	/*
	This function that return chat pointer.
	input: a chat id.
	output: chat pointer.
	*/
	virtual SingleChat* loadChat(unsigned int id) = 0;

	/*
	This function adds a group.
	input: a chat data and group chat data.
	output: none.
	*/
	virtual void addGroup(ChatData chatData, GroupChatData groupChatData) = 0;
	
	/*
	This function adds a user to chat.
	input: a username and group id.
	output: none.
	*/
	virtual bool addMemberToChat(UserInChat userInChat, int groupID) = 0;

	/*
	This function remove a user from group.
	input: a username and group id.
	output: none.
	*/
	virtual bool removeMemberFromGroup(User user, int groupId) = 0;
	
	/*
	This function give the last chat id.
	input: none.
	output: the last chat id.
	*/
	virtual int getLastChatId() = 0;

	/*
	This function returns the chat's names of the user
	input: username
	output: vector of chat's id
	*/
	virtual std::vector<unsigned int> getChatsIdsByUser(std::string username) = 0;

};