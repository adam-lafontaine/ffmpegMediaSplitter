#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace split 
{
	void split_multiple(
		fs::path const& ffmpeg_exe_dir,
		std::vector<fs::path>& src_files,
		fs::path const& dst_dir,
		std::string const& dst_base_file,
		std::string const& file_ext,
		unsigned const segment_sec);

}
