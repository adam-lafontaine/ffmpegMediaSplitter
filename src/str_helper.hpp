#pragma once

#include <string>

namespace str_helper 
{

	// returns true if a string is empty or only contains whitespace
	bool str_is_blank(std::string const& str);

	// returns true if a string represents and unsigned int
	bool str_is_unsigned(std::string const& str);

	// checks if a string ends with another string
	bool str_ends_with(std::string const& full_string, std::string const& end);

	// wraps a string in quotes
	std::string quoted(std::string const& str);

}
