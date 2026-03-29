#include "JsonResponsePacketSerializer.h"
#include <cstddef>
#include "Helper.h"

const Buffer JsonResponsePacketSerializer::serializeResponse(const ErrorResponse resp)
{
	Buffer buff;
	std::string msgError = "";

	// dump the message to json
	json jMsg = { {"message", resp.message} };
	msgError = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(ERROR_RESPONSE_CODE));

	buildResponseMsg(buff, msgError);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(const LoginResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(LOGIN_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(const SignupResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(SIGNUP_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const void JsonResponsePacketSerializer::buildResponseMsg(Buffer& buff, const std::string msg)
{
	for (char ch : msg)
	{
		buff.push_back(static_cast<unsigned char>(ch));
	}
}


const Buffer JsonResponsePacketSerializer::serializeResponse(const LogoutResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(SIGNOUT_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(const CreateNewGroupChatResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status}, {"chatId", resp.chatId} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(CREATE_NEW_GROUP_CHAT_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(const CreateNewSingleChatResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status}, {"chatId", resp.chatId} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(CREATE_NEW_SINGLE_CHAT_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(LoadChatResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json messages_js = resp.messages;
	json jMsg = { {"status", resp.status}, {"messages", messages_js} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(LOAD_CHAT_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}


const Buffer JsonResponsePacketSerializer::serializeResponse(const SendMessageResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(SEND_MESSAGE_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}


const Buffer JsonResponsePacketSerializer::serializeResponse(const ReciveMessageResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json msg_js = resp.msg;
	json jMsg = { {"status", resp.status}, {"message", msg_js}};
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(RECIVE_MESSAGE_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}


const Buffer JsonResponsePacketSerializer::serializeResponse(const RemoveUserResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status}, {"chatId", resp.chatId} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(REMOVE_USER_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}


const Buffer JsonResponsePacketSerializer::serializeResponse(const AddUserResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(ADD_USER_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(const GetAllUsersResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json users_js = resp.users;
	json jMsg = { {"status", resp.status}, {"users", users_js} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(GET_ALL_USERS_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(const GetUserChatsResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// create json of the chats
	json chats;
	for (auto pair : resp.chats)
	{
		chats[std::to_string(pair.first)] = pair.second;
	}

	// dump the message to json
	json jMsg = { {"status", resp.status}, {"chats", chats} };
	msg = jMsg.dump();

	// get the response code
	buff.push_back(static_cast<unsigned char>(GET_USERS_CHATS_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

const Buffer JsonResponsePacketSerializer::serializeResponse(const UpdateCreateNewChatResponse resp)
{
	Buffer buff;
	std::string msg = "";

	// dump the message to json
	json jMsg = { {"status", resp.status} , {"chatId", resp.chatId} , {"chatName", resp.chatName} };
	msg = jMsg.dump();

	// get the length of the msg
	std::string lengthStr = std::to_string(msg.length());

	// get the response code
	buff.push_back(static_cast<unsigned char>(UPDATE_CREATE_NEW_CHAT_RESPONSE_CODE));

	buildResponseMsg(buff, msg);

	return buff;
}

void to_json(json& j, const Message& msg)
{
	j = 
	{
		{"content", msg.content},
		{"sender", msg.sender.username},
		{"chatId", msg.chatId},
		{"time", Helper::toIso8601UTC(msg.time)},
		{"isTxtMsg", msg.isTxtMsg}
	};
}


void to_json(json& j, const ChatData& chatData)
{
	j =
	{
		{"id", chatData.id},
		{"createdAt", Helper::toIso8601UTC(chatData.createdAt)},
		{"chatType", chatData.chatType}
	};
}

void to_json(json& j, const User& user)
{
	j = { {"username", user.username} };
}
