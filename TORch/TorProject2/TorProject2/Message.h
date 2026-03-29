#pragma once
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "LoggedUser.h"

struct User;

typedef struct Message
{
	std::string content;
	User sender;
	unsigned int chatId;
	std::time_t time; // UTC
	bool isTxtMsg;
} Message;