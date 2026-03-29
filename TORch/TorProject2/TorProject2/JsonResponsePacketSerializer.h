#pragma once
#include <vector>
#include "json.hpp"
#include <string>
#include <iostream>
#include "Request.h"
#include "Response.h"


using json = nlohmann::json;

class JsonResponsePacketSerializer
{
public:

	// TODO: Add comments


	// This function serializes an error response to create a buffer.
	static const Buffer serializeResponse(const ErrorResponse resp);

	// This function serializes a login response to create a buffer.
	static const Buffer serializeResponse(const LoginResponse resp);

	// This function serializes a signup response to create a buffer.
	static const Buffer serializeResponse(const SignupResponse resp);

	// This function serializes a logout response to create a buffer.
	static const Buffer serializeResponse(const LogoutResponse resp);

	// This function serializes a create new chat response to create a buffer.
	static const Buffer serializeResponse(const CreateNewGroupChatResponse resp);

	// This function serializes a create new chat response to create a buffer.
	static const Buffer serializeResponse(const CreateNewSingleChatResponse resp);

	// This function serializes a load chat response to create a buffer.
	static const Buffer serializeResponse(LoadChatResponse resp);

	// This function serializes a send message response to create a buffer.
	static const Buffer serializeResponse(const SendMessageResponse resp);

	// This function serializes a recive message response to create a buffer.
	static const Buffer serializeResponse(const ReciveMessageResponse resp);

	// This function serializes a remove user response to create a buffer.
	static const Buffer serializeResponse(const RemoveUserResponse resp);

	// This function serializes a add user response to create a buffer.
	static const Buffer serializeResponse(const AddUserResponse resp);

	// This function serializes a get all users response to create a buffer.
	static const Buffer serializeResponse(const GetAllUsersResponse resp);

	// This function serializes a get chats names response to create a buffer.
	static const Buffer serializeResponse(const GetUserChatsResponse resp);

	// This function serializes a update users about new chat response to create a buffer.
	static const Buffer serializeResponse(const UpdateCreateNewChatResponse resp);

	// This function builds a response msg on a buffer it is given.
	static const void buildResponseMsg(Buffer& buff, const std::string msg);



};

	// to_json methods
	void to_json(json& j, const Message& msg);
	void to_json(json& j, const ChatData& data);
	void to_json(json& j, const User& user);
