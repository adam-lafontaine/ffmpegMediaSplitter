#include<filesystem>
#include<exception>
#include<iostream>
#include<string>
#include<math.h>
#include<vector>
#include<algorithm>
#include<chrono>

/*
LICENSE INFO:
Free to use for anyone who finds it.
*/

namespace fs = std::filesystem; // c++17
namespace chrono = std::chrono;

//====== DEFAULTS ====================
static std::string in_file_ext = ".mp3";
static std::string in_src_dir = fs::current_path().string();
static std::string in_base_name = "split";
static std::string in_dst_dir = in_src_dir + "\\" + in_base_name;
static unsigned in_segment_sec = 600;

// Windows file path
// Or add ffmpeg to PATH and change ffmpeg_ext_dir to "ffmpeg"
// Should work for Linux as well (not tested)
static std::string ffmpeg_exe_dir = "C:\\ffmpeg\\ffmpeg-4.2.1-win64-static\\bin\\";

// forward declare
void split_multiple(std::vector<std::string>& src_files, std::string const& dst_dir, std::string const& dst_base_file, unsigned segment_sec);
std::vector<std::string> get_files_of_type(std::string const& src_dir, std::string& extension);
void get_inputs();
void show_inputs();


int main() {
	bool prompt = true;
	bool quit = false;
	std::string selection;

	try
	{
		while (prompt && !quit) {
			get_inputs();

			std::cout << "\n";
			show_inputs();

			std::cout << "\n";

			int c;
			do {
				std::cout << "OK? (y/n), Quit (q): ";

				std::getline(std::cin, selection);
				c = tolower(selection[0]);

			} while (c != 'y' && c != 'n' && c != 'q');	
			
			switch (c)
			{
			case 'q':
				quit = true;
				break;
			case 'y':
				prompt = false;
				break;

			default:
				std::cout << "\n";
				break;
			}

		}		

		if (quit)
			return EXIT_SUCCESS;

		std::cout << "\n";
		
		auto file_list = get_files_of_type(in_src_dir, in_file_ext);
		fs::create_directories(in_dst_dir);
		split_multiple(file_list, in_dst_dir, in_base_name, in_segment_sec);
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}

}

//===================================

// display the inputs that the user chose/confirmed
void show_inputs() {
	std::cout << "          ffmpeg location: " << ffmpeg_exe_dir << "\n";
	std::cout << "           file extension: " << in_file_ext << "\n";
	std::cout << "            file location: " << in_src_dir << "\n";
	std::cout << "            new file name: " << in_base_name << "\n";
	std::cout << "        new file location: " << in_dst_dir << "\n";
	std::cout << "file segment length (sec): " << in_segment_sec << "\n";
}

// returns true if a string is empty or only contains whitespace
bool str_is_blank(std::string const& str) {
	return str.empty() || std::all_of(str.begin(), str.end(), isspace);
}

// returns true if a string represents and unsigned int
bool str_is_unsigned(std::string const& str) {
	return std::all_of(str.begin(), str.end(), isdigit);
}

// checks if a string ends with another string
bool str_ends_with(std::string const& full_string, std::string const& end) {
	if (full_string.length() < end.length())
		return false;

	return end.length() >= full_string.length() &&
		full_string.compare(full_string.length() - end.length(), end.length(), end) == 0;

}

// appends sub file/directory to a directory path string
std::string str_append_sub(std::string const& parent_dir, std::string const& sub) {
	constexpr auto slash = "\\";
	return str_ends_with(parent_dir, slash) ? parent_dir + sub : parent_dir + slash + sub;
}

// prompt the user for required inputs
void get_inputs() {

	std::string str;

	std::cout << "Enter ffmpeg location (" << ffmpeg_exe_dir << "):\n";
	std::getline(std::cin, str);
	if (!str_is_blank(str) && fs::exists(str))
		ffmpeg_exe_dir = str;

	std::cout << "Enter file extension (" << in_file_ext <<"): ";
	std::getline(std::cin, str);
	if (!str_is_blank(str))
		in_file_ext = str;

	std::cout << "Enter file location (" << in_src_dir << "):\n";
	std::getline(std::cin, str);
	if (!str_is_blank(str) && fs::exists(str))
		in_src_dir = str;

	std::cout << "Enter new file name (" << in_base_name << "): ";
	std::getline(std::cin, str);
	if (!str_is_blank(str))
		in_base_name = str;

	in_dst_dir = str_append_sub(in_src_dir, in_base_name);

	std::cout << "Enter new file location (" << in_dst_dir << "):\n";
	std::getline(std::cin, str);
	if (!str_is_blank(str))
		in_dst_dir = str;

	std::cout << "Enter file segment length in seconds (" << in_segment_sec << "): ";
	std::getline(std::cin, str);
	if (!str_is_blank(str) && str_is_unsigned(str))
		in_segment_sec = atoi(str.c_str());	

}

// returns console output of a system command as a string
std::string out_from_command(std::string cmd) {

	std::string data;
	FILE* stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	cmd.append(" 2>&1");

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
double get_seconds(std::string const& file_path) {
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
void split_single(std::string const& src_file_path, std::string const& dst_full_path_base, unsigned segment_sec) {
	// ffmpeg -i "input_audio_file.mp3" -f segment -segment_time 3600 -c copy output_audio_file_%03d.mp3

	auto const src_duration = get_seconds(src_file_path);
	auto const num_out_files = num_split_files(src_duration, segment_sec);
	auto const digits = num_digits(num_out_files);

	auto const command = ffmpeg_exe_dir + "ffmpeg -i " + "\"" + src_file_path + "\""
		+ " -f segment -segment_time " + std::to_string(segment_sec)
		+ " -c copy " + +"\"" + dst_full_path_base + "\"" + "_%0" + std::to_string(digits) + "d.mp3";

	system(command.c_str());
}


// splits a number of files into chunks of a given duration
void split_multiple(std::vector<std::string>& src_files, std::string const& dst_dir, std::string const& dst_base_file, unsigned segment_sec) {
	if (src_files.empty())
		return;
	
	// sort alphabetically
	std::sort(src_files.begin(), src_files.end());

	// get max length of index number
	unsigned idx_len = num_digits(src_files.size());

	unsigned idx = 1;
	char idx_str[100];

	// timestamp used for temp file names
	auto const ms = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

	// split each file with temp names
	// ffmpeg naming scheme is zero based
	auto const temp_tag = std::to_string(ms) + "_temp_";
	auto const temp_path_base = str_append_sub(dst_dir, temp_tag);

	auto split_file = [&](std::string const& file_path) {
		sprintf_s(idx_str, "%0*d", idx_len, idx++); // zero pad index number
		auto temp_path = temp_path_base + idx_str;
		split_single(file_path, temp_path, segment_sec);
		memset(idx_str, 0, strlen(idx_str));
	};

	std::for_each(src_files.begin(), src_files.end(), split_file);


	//for (auto const& file_path : src_files) {
	//	sprintf_s(idx_str, "%0*d", idx_len, idx++); // zero pad index number

	//	auto temp_path = temp_path_base + idx_str;
	//	split_single(file_path, temp_path, segment_sec);

	//	memset(idx_str, 0, strlen(idx_str));
	//}

	auto const entry_match = [&](fs::path const& entry) {
		return fs::is_regular_file(entry) &&
			entry.has_extension() &&
			entry.extension() == in_file_ext &&
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
	auto const base_dir = str_append_sub(dst_dir, dst_base_file);

	auto const rename_file = [&](std::string const& file_path) {
		sprintf_s(idx_str, "%0*d", idx_len, idx++);
		auto const new_path = base_dir + "_" + idx_str + in_file_ext;
		auto result = rename(file_path.c_str(), new_path.c_str());
		memset(idx_str, 0, strlen(idx_str));
	};

	std::for_each(file_list.begin(), file_list.end(), rename_file);


	/*for (auto const& file_path : file_list) {
		sprintf_s(idx_str, "%0*d", idx_len, idx++);

		auto new_path = base_dir + "_" + idx_str + in_file_ext;

		auto result = rename(file_path.c_str(), new_path.c_str());

		memset(idx_str, 0, strlen(idx_str));
	}*/

}

// returns all files in a directory with a given extension
// be careful pre c++11
std::vector<std::string> get_files_of_type(std::string const& src_dir, std::string& extension) {

	// extenstion must begin with '.'
	if (extension[0] != '.')
		extension = "." + extension;

	std::vector<std::string> file_list;

	auto const entry_match = [&](fs::path const& entry) {
		return fs::is_regular_file(entry) &&
			entry.has_extension() &&
			entry.extension() == extension;
	};
	
	for (auto const& entry : fs::directory_iterator(src_dir)) {
		if(entry_match(entry))
			file_list.push_back(entry.path().string());		
	}

	return file_list;
}