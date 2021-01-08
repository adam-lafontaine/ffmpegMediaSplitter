#include "convert.hpp"
#include "str_helper.hpp"

#include<algorithm>
#include <chrono>

namespace chrono = std::chrono;
namespace str = str_helper;


// gets the number of digits in an unsigned int
static unsigned num_digits(unsigned const number) {

	return static_cast<unsigned>(std::to_string(number).length());
}


static void convert_single(
	fs::path const& ffmpeg_exe_dir,
	fs::path const& src_file_path,
	fs::path const& dst_file_path)
{
	// ffmpeg -i "C:\path\to\input.m4b" -acodec libmp3lame -ar 22050 "C:\path\to\output.mp3"

	auto const command = 
		ffmpeg_exe_dir.string() 
		+ "ffmpeg -i " 
		+ str::quoted(src_file_path.string())
		+ " -acodec libmp3lame -ar 22050 " 
		+ str::quoted(dst_file_path.string());

	system(command.c_str());
}


namespace convert 
{
	void convert_multiple(
		fs::path const& ffmpeg_exe_dir,
		std::vector<fs::path>& src_files,
		fs::path const& dst_dir,
		std::string const& out_ext)
	{
		if (src_files.empty())
			return;

		// sort alphabetically
		std::sort(src_files.begin(), src_files.end());

		// get max length of index number
		auto const file_id_len = num_digits(src_files.size());

		unsigned file_id = 1;
		char file_id_str[100];

		// timestamp used for temp file names
		auto const ms = std::to_string(
			chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()
		);

		auto const file_name_base = "ffmpeg_" + ms.substr(ms.length() - 5) + "_convert_";

		for (auto& file_path : src_files) 
		{
			sprintf_s(file_id_str, "%0*d", file_id_len, file_id++); // zero pad index number
			auto const file_name = file_name_base + file_id_str + out_ext;
			auto const out_file_path = dst_dir / file_name;

			convert_single(ffmpeg_exe_dir, file_path, out_file_path);

			file_path = out_file_path; // rename file in list
		}

	}
}
