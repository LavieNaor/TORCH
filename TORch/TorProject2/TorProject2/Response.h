#pragma once
#include "LoggedUser.h"
#include "SingleChat.h"
#include "Message.h"
#include <vector>
#include <string>

typedef std::vector<unsigned char> Buffer;


typedef struct ErrorResponse
{
	std::string message;
} ErrorResponse;


typedef struct LoginResponse
{
	unsigned int status;
} LoginResponse;


typedef struct SignupResponse
{
	unsigned int status;
} SignupResponse;


typedef struct LogoutResponse
{
	unsigned int status;
} LogoutResponse;

typedef struct GetAllUsersResponse
{
	unsigned int status;
	std::vector<User> users;
} GetAllUsersResponse;

typedef struct CreateNewGroupChatResponse
{
	unsigned int status;
	unsigned int chatId;
} CreateNewGroupChatResponse;

typedef struct CreateNewSingleChatResponse
{
	unsigned int status;
	unsigned int chatId;
} CreateNewSingleChatResponse;

typedef struct UpdateCreateNewChatResponse
{
	unsigned int status;
	unsigned int chatId;
	std::string chatName;
} UpdateCreateNewChatResponse;

typedef struct LoadChatResponse
{
	unsigned int status;
	std::vector<Message> messages;
} LoadChatResponse;

typedef struct SendMessageResponse
{
	unsigned int status;
} SendMessageResponse;

typedef struct ReciveMessageResponse
{
	unsigned int status;
	Message msg;
} ReciveMessageResponse;

typedef struct RemoveUserResponse
{
	unsigned int status;
	unsigned int chatId;
} RemoveUserResponse;

typedef struct AddUserResponse
{
	unsigned int status;
} AddUserResponse;

typedef struct GetUserChatsResponse
{
	unsigned int status;
	std::map<unsigned int, std::string> chats; // {"id": "chatName"}
} GetUserChatsResponse;