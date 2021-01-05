#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace convert {

	void convert_multiple(
		fs::path const& ffmpeg_exe_dir,
		std::vector<fs::path>& src_files,
		fs::path const& dst_dir,
		const char* out_ext);
}