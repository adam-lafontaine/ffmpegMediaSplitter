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
		fs::path const& ffmpeg_exe_dir,
		fs::path const& src_file_path,
		fs::path const& dst_file_path)
	{
		// ffmpeg -i "C:\path\to\input.m4b" -acodec libmp3lame -ar 22050 "C:\path\to\output.mp3"

		auto const command = ffmpeg_exe_dir.string() + "ffmpeg -i " + "\"" + src_file_path.string() + "\""
			+ " -acodec libmp3lame -ar 22050 " + "\"" + dst_file_path.string() + "\"";

		system(command.c_str());
	}

	void convert_multiple(
		fs::path const& ffmpeg_exe_dir,
		std::vector<fs::path>& src_files,
		fs::path const& dst_dir,
		const char* out_ext)
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

		auto const file_name_base = "ffmpeg_" + ms.substr(ms.length() - 5) + "_convert_";

		for (auto& file_path : src_files) 
		{
			sprintf_s(idx_str, "%0*d", idx_len, idx++); // zero pad index number
			auto const file_name = file_name_base + idx_str + out_ext;
			auto const out_file_path = dst_dir / file_name;

			convert_single(ffmpeg_exe_dir, file_path, out_file_path);

			file_path = out_file_path; // rename file in list
		}


	}
}
