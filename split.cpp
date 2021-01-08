#include "split.hpp"
#include "str_helper.hpp"
#include "platform.hpp"

#include <filesystem>
#include <chrono>
#include <algorithm>


namespace fs = std::filesystem;
namespace chrono = std::chrono;
namespace str = str_helper;
namespace pfm = platform;

// returns console output of a system command as a string


// gets the duration in seconds from a media file
static double get_seconds(std::string const& ffmpeg_exe_dir, std::string const& file_path) {
	// ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 in_file.mp3

	constexpr auto cmd = "ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 ";

	auto const command = ffmpeg_exe_dir + cmd + str::quoted(file_path);

	return atof(pfm::get_command_output(command).c_str());
}


// gets the number of digits in an unsigned int
static unsigned num_digits(unsigned const number) {

	return static_cast<unsigned>(std::to_string(number).length());
}

// determines the number files a source file needs to be split into
static unsigned num_split_files(double const src_duration, double const split_duration) {

	if (src_duration <= split_duration)
		return 1;

	auto const amount = src_duration / split_duration;
	double fractpart, intpart;

	fractpart = modf(amount, &intpart);

	return (unsigned)intpart + (unsigned)(fractpart > 0);
}


// splits a media file into chunks of a given duration
static void split_single(
	fs::path const& ffmpeg_exe_dir,
	fs::path const& src_file_path,
	fs::path const& dst_full_path_base,
	std::string const& file_ext,
	unsigned segment_sec)
{
	// ffmpeg -i "input_audio_file.mp3" -f segment -segment_time 3600 -c copy output_audio_file_%03d.mp3

	auto const src_duration = get_seconds(ffmpeg_exe_dir.string(), src_file_path.string());
	auto const num_out_files = num_split_files(src_duration, segment_sec);
	auto const digits = num_digits(num_out_files);

	auto const command =
		ffmpeg_exe_dir.string()
		+ "ffmpeg -i "
		+ str::quoted(src_file_path.string())
		+ " -f segment -segment_time "
		+ std::to_string(segment_sec)
		+ " -c copy "
		+ str::quoted(dst_full_path_base.string())
		+ "_%0" + std::to_string(digits)
		+ "d"
		+ file_ext;

	system(command.c_str());
}


namespace split 
{
	// splits a number of files into chunks of a given duration
	void split_multiple(
		fs::path const& ffmpeg_exe_dir,
		std::vector<fs::path>& src_files,
		fs::path const& dst_dir,
		std::string const& dst_base_file,
		std::string const& file_ext,
		unsigned const segment_sec)
	{
		if (src_files.empty())
			return;

		// sort alphabetically
		std::sort(src_files.begin(), src_files.end());

		// get max length of index number
		auto file_id_len = num_digits(src_files.size());

		unsigned file_id = 1;
		char file_id_str[100];

		// timestamp used for temp file names
		auto const ms = std::to_string(
			chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()
		);

		// split each file with temp names
		// ffmpeg naming scheme is zero based
		auto const file_name_base = "ffmpeg_" + ms.substr(ms.length() - 5) + "_temp_";

		for (auto const& file_path : src_files) 
		{
			sprintf_s(file_id_str, "%0*d", file_id_len, file_id++); // zero pad index number
			auto const file_name = file_name_base + file_id_str;
			auto const temp_path = dst_dir /  file_name;
			split_single(ffmpeg_exe_dir, file_path, temp_path, file_ext, segment_sec);
			memset(file_id_str, 0, strlen(file_id_str));
		}

		auto const entry_match = [&](fs::path const& entry) 
		{
			return fs::is_regular_file(entry) &&
				entry.has_extension() &&
				entry.extension() == file_ext &&
				entry.filename().string()._Starts_with(file_name_base);
		};

		// get all of the files created
		std::vector<fs::path> file_list;
		for (auto const& entry : fs::directory_iterator(dst_dir)) 
		{
			if (entry_match(entry))
				file_list.push_back(entry.path());
		}

		// sort alphabetically
		std::sort(file_list.begin(), file_list.end());

		// set track numbers and rename
		file_id_len = num_digits(file_list.size());
		file_id = 1;

		auto const create_tracks = [&](fs::path const& file_path)
		{
			// ffmpeg -i in.mp3 -metadata track="1/12" out.mp3

			auto const track_part = " -metadata track=" + str::quoted( std::to_string(file_id) + "/" + std::to_string(file_list.size()) );				

			sprintf_s(file_id_str, "%0*d", file_id_len, file_id++);

			auto const file_name = dst_base_file + "_" + file_id_str + file_ext;

			auto const new_path = (dst_dir / file_name).string();

			memset(file_id_str, 0, strlen(file_id_str));

			auto const command =
				ffmpeg_exe_dir.string() 
				+ "ffmpeg -i " 
				+ str::quoted(file_path.string())
				+ track_part
				+ str::quoted(new_path);

			// create new file with track number and new name
			system(command.c_str());

			// delete temp file
			fs::remove(file_path);
		};

		std::for_each(file_list.begin(), file_list.end(), create_tracks);
	}

}