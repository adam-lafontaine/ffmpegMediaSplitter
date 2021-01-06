#include "str_helper.hpp"

#include<algorithm>

namespace str_helper 
{

	// returns true if a string is empty or only contains whitespace
	bool str_is_blank(std::string const& str) 
	{
		return str.empty() || std::all_of(str.begin(), str.end(), isspace);
	}

	// returns true if a string represents and unsigned int
	bool str_is_unsigned(std::string const& str) 
	{
		return std::all_of(str.begin(), str.end(), isdigit);
	}

	// checks if a string ends with another string
	bool str_ends_with(std::string const& full_string, std::string const& end) 
	{
		return end.length() <= full_string.length() &&
			full_string.compare(full_string.length() - end.length(), end.length(), end) == 0;
	}


	std::string quoted(std::string const& str)
	{
		return std::string("\"") + str + "\"";
	}

}