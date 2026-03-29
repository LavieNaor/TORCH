#pragma once
#include <vector>
#include "json.hpp"
#include <string>
#include <iostream>
#include "Request.h"

using json = nlohmann::json;


class JsonRequestPacketDeserializer
{
public:

	// TODO: Add comments


	// This function deserializes a buffer to get a login requests.
	static const LoginRequest deserializeLoginRequest(const Buffer buff);

	// This function deserializes a buffer to get a signup requests.
	static const SignupRequest deserializeSignupRequest(const Buffer buff);

	// This function deserializes a buffer to get a create ne chat requests.
	static const CreateNewGroupChatRequest deserializeCreateNewGroupChatRequest(const Buffer buff);

	// This function deserializes a buffer to get a create ne chat requests.
	static const CreateNewSingleChatRequest deserializeCreateNewSingleChatRequest(const Buffer buff);

	// This function deserializes a buffer to get a load chat requests.
	static const LoadChatRequest deserializeLoadChatRequest(const Buffer buff);

	// This function deserializes a buffer to get a send message requests.
	static const SendMessageRequest deserializeSendMessageRequest(const Buffer buff);

	// This function deserializes a buffer to get a add user requests.
	static const AddUserRequest deserializeAddUserRequest(const Buffer buff);

	// This function deserializes a buffer to get a remove user requests.
	static const RemoveUserRequest deserializeRemoveUserRequest(const Buffer buff);

	// This function deserializes a buffer to get chats names requests.
	static const GetChatsNameRequest deserializeGetChatsNameRequest(const Buffer buff);
};


// from_json methods
void from_json(const json& j, User& u);
void from_json(const json& j, Message& u);