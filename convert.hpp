#pragma once

#include <string>
#include <vector>

namespace convert {

	void convert_multiple(
		std::string const& ffmpeg_exe_dir,
		std::vector<std::string>& src_files,
		std::string const& dst_dir,
		std::string const& out_ext);
}