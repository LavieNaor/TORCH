#pragma once
#include <string>
#include <stdexcept>
#include <vector>
#include "json.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

class Helper
{
public:

	// This function transform str to unsigned int
	static unsigned int stoui(const std::string& s);

	// transform from 8601UTC format to time_t
	static std::time_t fromIso8601UTC(const std::string& iso);

	// transform from time_t to 8601UTC format
	static std::string toIso8601UTC(std::time_t t);

};
