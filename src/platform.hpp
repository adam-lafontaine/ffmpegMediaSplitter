#pragma once

#include <string>

namespace platform
{
	// returns console output of a system command as a string
	std::string get_command_output(std::string const& command);
}

