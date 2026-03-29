#include "JsonRequestPacketDeserializer.h"
#include "Helper.h"

const LoginRequest JsonRequestPacketDeserializer::deserializeLoginRequest(const Buffer buff)
{
	LoginRequest req;
	std::string inf(buff.begin(), buff.end());

	auto ob = json::parse(inf);
	if (ob.size() < 2)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 2)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		req.username = ob["username"];
		req.password = ob["password"];
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}

const SignupRequest JsonRequestPacketDeserializer::deserializeSignupRequest(const Buffer buff)
{
	SignupRequest req;
	std::string inf(buff.begin(), buff.end());

	auto ob = json::parse(inf);
	if (ob.size() < 3)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 3)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		req.username = ob["username"];
		req.password = ob["password"];
		req.email = ob["email"];
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}

const CreateNewGroupChatRequest JsonRequestPacketDeserializer::deserializeCreateNewGroupChatRequest(const Buffer buff)
{
	CreateNewGroupChatRequest req;
	std::string inf(buff.begin(), buff.end());

	auto ob = json::parse(inf);
	if (ob.size() < 2)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 2)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		// get a vector of users with from_json function
		req.users = ob["users"].get<std::vector<User>>();
		req.groupName = ob["groupName"];
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}

const CreateNewSingleChatRequest JsonRequestPacketDeserializer::deserializeCreateNewSingleChatRequest(const Buffer buff)
{
	CreateNewSingleChatRequest req;
	std::string inf(buff.begin(), buff.end());

	auto ob = json::parse(inf);
	if (ob.size() < 1)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 1)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		User userToCreateChatWith;
		userToCreateChatWith.username = ob["username"];
		req.users.push_back(userToCreateChatWith);
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}

const LoadChatRequest JsonRequestPacketDeserializer::deserializeLoadChatRequest(const Buffer buff)
{
	LoadChatRequest req;
	std::string inf(buff.begin(), buff.end());

	auto ob = json::parse(inf);
	if (ob.size() < 1)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 1)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		req.chatID = ob["chatID"].get<unsigned int>();
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}


const SendMessageRequest JsonRequestPacketDeserializer::deserializeSendMessageRequest(const Buffer buff)
{
	SendMessageRequest req;
	std::string inf(buff.begin(), buff.end());
	User sender;

	auto ob = json::parse(inf);
	if (ob.size() < 5)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 5)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		sender.username = ob["sender"];

		// transform string to unsigned int
		req.msg.chatId = ob["chatId"].get<unsigned int>();
		req.msg.content = ob["content"];
		req.msg.isTxtMsg = ob["isTxtMsg"].get<int>();
		req.msg.sender = sender;
		req.msg.time = Helper::fromIso8601UTC(ob["time"]); // from utc format to time_t
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}


const AddUserRequest JsonRequestPacketDeserializer::deserializeAddUserRequest(const Buffer buff)
{
	AddUserRequest req;
	std::string inf(buff.begin(), buff.end());

	std::vector<std::string> usernamesToAddVecStr;
	std::vector<User> usersToAddVec;

	auto ob = json::parse(inf);
	if (ob.size() < 2)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 2)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		// transform string to unsigned int
		req.chatID = ob["chatID"].get<unsigned int>();

		// usernames string vector
		usernamesToAddVecStr = ob["usernames_to_add"];

		// build vector of user to add
		for (auto it = usernamesToAddVecStr.begin(); it != usernamesToAddVecStr.end(); it++)
		{
			usersToAddVec.push_back(User(*it));
		}

		// set the usernames in the request
		req.usernames = usersToAddVec;
	}

	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}

const RemoveUserRequest JsonRequestPacketDeserializer::deserializeRemoveUserRequest(const Buffer buff)
{
	RemoveUserRequest req;
	std::string inf(buff.begin(), buff.end());

	auto ob = json::parse(inf);
	if (ob.size() < 2)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 2)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		// transform string to unsigned int
		req.chatID = ob["chatID"].get<unsigned int>();
		req.username = ob["username_to_remove"];
		
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}

const GetChatsNameRequest JsonRequestPacketDeserializer::deserializeGetChatsNameRequest(const Buffer buff)
{
	GetChatsNameRequest req;
	std::string inf(buff.begin(), buff.end());

	auto ob = json::parse(inf);
	if (ob.size() < 1)
	{
		throw std::runtime_error("Not Enough Arguments.");
	}
	else if (ob.size() > 1)
	{
		throw std::runtime_error("Too many Arguments.");
	}
	try
	{
		req.username = ob["username"];
	}
	catch (std::exception e)
	{
		throw std::runtime_error("Non Valid Arguments.");
	}
	return req;
}


void from_json(const json& j, User& u)
{
	j.at("username").get_to(u.username);
}

std::vector<User> parseUsers(const std::string& jsonStr)
{
	json j = json::parse(jsonStr);

	std::vector<User> users;

	for (const auto& u : j["users"])
	{
		User user;
		user.username = u["username"];
		users.push_back(user);
	}

	return users;
}

void from_json(const json& j, Message& u)
{
	j.at("content").get_to(u.content);
	j.at("sender").get_to(u.sender.username);
	j.at("chatId").get_to(u.chatId);
	j.at("isTxtMsg").get_to(u.isTxtMsg);

	// write the time in time_t variable
	std::string ts;
	j.at("time").get_to(ts);
	u.time = Helper::fromIso8601UTC(ts);
}
