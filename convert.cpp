#include<algorithm>
#include <chrono>

#include "convert.hpp"
#include "str_helper.hpp"

namespace convert {

	namespace chrono = std::chrono;
	namespace str = str_helper;

	// gets the number of digits in an unsigned int
	unsigned num_digits(unsigned const number) {

		char buffer[100];
		sprintf_s(buffer, "%d", number);
		return strlen(buffer);
	}

	void convert_single(
		std::string const& ffmpeg_exe_dir,
		std::string const& src_file_path,
		std::string const& dst_file_path)
	{
		// ffmpeg -i "C:\path\to\input.m4b" -acodec libmp3lame -ar 22050 "C:\path\to\output.mp3"

		auto const command = ffmpeg_exe_dir + "ffmpeg -i " + "\"" + src_file_path + "\""
			+ " -acodec libmp3lame -ar 22050 " + "\"" + dst_file_path + "\"";

		system(command.c_str());
	}

	void convert_multiple(
		std::string const& ffmpeg_exe_dir,
		std::vector<std::string>& src_files,
		std::string const& dst_dir,
		std::string const& out_ext)
	{
		if (src_files.empty())
			return;

		// sort alphabetically
		std::sort(src_files.begin(), src_files.end());

		// get max length of index number
		unsigned idx_len = num_digits(src_files.size());

		unsigned idx = 1;
		char idx_str[100];

		// timestamp used for temp file names
		auto const ms = std::to_string(
			chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()
		);

		auto const tag = "ffmpeg_" + ms.substr(ms.length() - 5) + "_convert_";
		auto const path_base = str::str_append_sub(dst_dir, tag);

		for (auto& file_path : src_files) {
			sprintf_s(idx_str, "%0*d", idx_len, idx++); // zero pad index number
			auto const out_file_path = path_base + idx_str + out_ext;

			convert_single(ffmpeg_exe_dir, file_path, out_file_path);

			file_path = out_file_path; // rename file in list
		}


	}
}