#include "platform.hpp"


static FILE* open_stream(const char* cmd)
{

#ifdef _WIN32

	return _popen(cmd, "r");

#endif

#ifdef linux

	return popen(cmd, "r");

#endif

}


static void close_stream(FILE* stream)
{

#ifdef _WIN32

	_pclose(stream);

#endif

#ifdef linux

	pclose(stream);

#endif

}


namespace platform
{
	std::string get_command_output(std::string const& command)
	{
		std::string data;
		FILE* stream;
		const int max_buffer = 256;
		char buffer[max_buffer];
		auto const cmd = command + " 2>&1";

		stream = open_stream(cmd.c_str());

		if (!stream)
			return data;

		while (!feof(stream))
		{
			if (fgets(buffer, max_buffer, stream) != NULL) 
				data.append(buffer);
		}

		close_stream(stream);

		return data;
	}
}


