#include<algorithm>
#include "str_helper.hpp"

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


	

	// appends sub file/directory to a directory path string
	/*std::string str_append_sub(std::string const& parent_dir, std::string const& sub) 
	{
		constexpr auto slash = R"(\)";
		return str_ends_with(parent_dir, slash) ? parent_dir + sub : parent_dir + slash + sub;
	}*/

}