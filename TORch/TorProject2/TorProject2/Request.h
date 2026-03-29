#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "IRequestHandler.h"
#include "Message.h"

class IRequestHandler;

typedef std::vector<unsigned char> Buffer;
typedef int RequestId;


typedef struct RequestInfo
{
	RequestId id;
	std::time_t receivalTime;
	Buffer buffer;
} RequestInfo;

typedef struct RequestResult
{
	Buffer response;
	IRequestHandler* newHandler;
} RequestResult;


typedef struct LoginRequest
{
	std::string username;
	std::string password;
} LoginRequest;

typedef struct SignupRequest
{
	std::string username;
	std::string password;
	std::string email;
} SignupRequest;

typedef struct CreateNewGroupChatRequest
{
	std::vector<User> users;
	std::string groupName;
} CreateNewGroupChatRequest;

typedef struct CreateNewSingleChatRequest
{
	std::vector<User> users;
} CreateNewSingleChatRequest;

typedef struct LoadChatRequest
{
	unsigned int chatID;
} LoadChatRequest;

typedef struct SendMessageRequest
{
	Message msg;
} SendMessageRequest;

typedef struct RemoveUserRequest
{
	std::string username;
	unsigned int chatID;
} RemoveUserRequest;

typedef struct AddUserRequest
{
	
	std::vector<User> usernames;
	unsigned int chatID;
} AddUserRequest;

typedef struct GetChatsNameRequest
{
	std::string username;
} GetChatsNameRequest;

