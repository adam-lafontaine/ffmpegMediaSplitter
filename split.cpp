#include <filesystem>
#include <chrono>

#include "split.hpp"
#include "str_helper.hpp"


namespace split {

	namespace fs = std::filesystem;
	namespace chrono = std::chrono;
	namespace str = str_helper;

	// returns console output of a system command as a string
	std::string out_from_command(std::string const& command) {

		std::string data;
		FILE* stream;
		const int max_buffer = 256;
		char buffer[max_buffer];
		auto const cmd = command + " 2>&1";

#ifdef _WIN32

		stream = _popen(cmd.c_str(), "r");
		if (stream) {
			while (!feof(stream))
				if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
			_pclose(stream);
		}

#endif

#ifdef linux
		// not tested
		stream = popen(cmd.c_str(), "r");
		if (stream) {
			while (!feof(stream))
				if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
			pclose(stream);
		}

#endif
		return data;
	}

	// gets the duration in seconds from a media file
	double get_seconds(std::string const& ffmpeg_exe_dir, std::string const& file_path) {
		// ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 in_file.mp3

		constexpr auto cmd = "ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 ";

		auto const command = ffmpeg_exe_dir + cmd + "\"" + file_path + "\"";

		return atof(out_from_command(command).c_str());
	}

	// gets the number of digits in an unsigned int
	unsigned num_digits(unsigned const number) {

		char buffer[100];
		sprintf_s(buffer, "%d", number);
		return strlen(buffer);
	}

	// determines the number files a source file needs to be split into
	unsigned num_split_files(double const src_duration, double const split_duration) {

		if (src_duration <= split_duration)
			return 1;

		auto const amount = src_duration / split_duration;
		double fractpart, intpart;

		fractpart = modf(amount, &intpart);

		return (unsigned)intpart + (unsigned)(fractpart > 0);
	}


	// splits a media file into chunks of a given duration
	void split_single(
		std::string const& ffmpeg_exe_dir,
		std::string const& src_file_path,
		std::string const& dst_full_path_base,
		std::string const& file_ext,
		unsigned segment_sec)
	{
		// ffmpeg -i "input_audio_file.mp3" -f segment -segment_time 3600 -c copy output_audio_file_%03d.mp3

		auto const src_duration = get_seconds(ffmpeg_exe_dir, src_file_path);
		auto const num_out_files = num_split_files(src_duration, segment_sec);
		auto const digits = num_digits(num_out_files);

		auto const command = ffmpeg_exe_dir + "ffmpeg -i " + "\"" + src_file_path + "\""
			+ " -f segment -segment_time " + std::to_string(segment_sec)
			+ " -c copy " + +"\"" + dst_full_path_base + "\"" + "_%0" + std::to_string(digits) + "d" + file_ext;

		system(command.c_str());
	}


	// splits a number of files into chunks of a given duration
	void split_multiple(
		std::string const& ffmpeg_exe_dir,
		std::vector<std::string>& src_files,
		std::string const& dst_dir,
		std::string const& dst_base_file,
		std::string const& file_ext,
		unsigned const segment_sec)
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

		// split each file with temp names
		// ffmpeg naming scheme is zero based
		auto const temp_tag = "ffmpeg_" + ms.substr(ms.length() - 5) + "_temp_";
		auto const temp_path_base = str::str_append_sub(dst_dir, temp_tag);

		auto split_file = [&](std::string const& file_path) {
			sprintf_s(idx_str, "%0*d", idx_len, idx++); // zero pad index number
			auto const temp_path = temp_path_base + idx_str;
			split_single(ffmpeg_exe_dir, file_path, temp_path, file_ext, segment_sec);
			memset(idx_str, 0, strlen(idx_str));
		};

		std::for_each(src_files.begin(), src_files.end(), split_file);


		auto const entry_match = [&](fs::path const& entry) {
			return fs::is_regular_file(entry) &&
				entry.has_extension() &&
				entry.extension() == file_ext &&
				entry.filename().string()._Starts_with(temp_tag);
		};

		// get all of the files created
		std::vector<std::string> file_list;
		for (auto const& entry : fs::directory_iterator(dst_dir)) {
			if (entry_match(entry))
				file_list.push_back(entry.path().string());
		}

		// sort alphabetically
		std::sort(file_list.begin(), file_list.end());

		// rename files
		idx_len = num_digits(file_list.size());
		idx = 1;
		auto const base_dir = str::str_append_sub(dst_dir, dst_base_file);

		auto const rename_file = [&](std::string const& file_path) {
			sprintf_s(idx_str, "%0*d", idx_len, idx++);
			auto const new_path = base_dir + "_" + idx_str + file_ext;
			auto const result = rename(file_path.c_str(), new_path.c_str());
			memset(idx_str, 0, strlen(idx_str));
		};

		std::for_each(file_list.begin(), file_list.end(), rename_file);

	}

}