	#pragma once
#include <vector>
#include <ctime>
#include "Request.h"

class IRequestHandler;

struct RequestInfo;
struct RequestResult;



enum message_request_codes
{
	SIGNUP_CODE = 1,
    LOGIN_CODE = 2,
	SIGNOUT_CODE = 3,
	CREATE_NEW_GROUP_CHAT_CODE = 4,
	CREATE_NEW_SINGLE_CHAT_CODE = 5,
	LOAD_CHAT_CODE = 6,
	SEND_MESSAGE_CODE = 7,
	GET_ALL_USERS_CODE = 8,
	ADD_USER_CODE = 9,
	REMOVE_USER_CODE = 10,
	// do not use code 11! - code 111 use for recive msg response
	GET_USERS_CHATS_CODE = 12
	// do not use code 13! - code 111 use for update create new chat response
};


enum message_response_codes
{
	ERROR_RESPONSE_CODE = 99,
	SIGNUP_RESPONSE_CODE = 101,
    LOGIN_RESPONSE_CODE = 102,
	SIGNOUT_RESPONSE_CODE = 103,
	CREATE_NEW_GROUP_CHAT_RESPONSE_CODE = 104,
	CREATE_NEW_SINGLE_CHAT_RESPONSE_CODE = 105,
	LOAD_CHAT_RESPONSE_CODE = 106,
	SEND_MESSAGE_RESPONSE_CODE = 107,
	GET_ALL_USERS_RESPONSE_CODE = 108,
	ADD_USER_RESPONSE_CODE = 109,
	REMOVE_USER_RESPONSE_CODE = 110,
	RECIVE_MESSAGE_RESPONSE_CODE = 111,
	GET_USERS_CHATS_RESPONSE_CODE = 112,
	UPDATE_CREATE_NEW_CHAT_RESPONSE_CODE = 113
};



class IRequestHandler
{
public:

	// The function checks if the request is relevent.
	virtual const bool isRequestRelevant(const RequestInfo inf) const = 0;

	// The function handles the request of a user.
	virtual const RequestResult handleRequest(const RequestInfo inf) = 0;
};