#pragma once

#include <string>
#include <vector>

namespace split {

	void split_multiple(
		std::string const& ffmpeg_exe_dir,
		std::vector<std::string>& src_files,
		std::string const& dst_dir,
		std::string const& dst_base_file,
		std::string const& file_ext,
		unsigned const segment_sec);

}
