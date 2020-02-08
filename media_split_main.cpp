#include<filesystem>
#include<exception>
#include<iostream>
#include<string>
#include<vector>


#include "app_config.hpp"
#include "str_helper.hpp"
#include "split.hpp"
#include "convert.hpp"

/*
LICENSE INFO:
Free to use for anyone who finds it.
*/

namespace fs = std::filesystem; // c++17

namespace str = str_helper;
namespace cvt = convert;

std::vector<std::string> get_files_of_type(std::string const& src_dir, std::string& extension);
void get_inputs();
void show_inputs();
bool prompt_user();
bool split_files();

//====== DEFAULTS USER INPUTS ====================

static std::string file_ext = MP3_EXT;
static std::string src_dir = fs::current_path().string();
static std::string base_name = FILE_NAME_BASE_DEFAULT;
static std::string dst_dir;
static unsigned segment_sec = SEGMENT_SEC_DEFAULT;
static std::string ffmpeg_exe_dir = FFMPEG_EXE_DIR;


int main() {

	if (!prompt_user())
		return EXIT_SUCCESS;

	if (!split_files())
		return EXIT_FAILURE;
}

//===================================

bool split_files() {

	try
	{
		auto file_list = get_files_of_type(src_dir, file_ext);
		fs::create_directories(dst_dir);

		auto out_file_ext = file_ext;
		bool converted = false;
		auto temp_dst = str::str_append_sub(dst_dir, "convert_temp");

		if (out_file_ext != MP3_EXT) { // try to convert everything to mp3
			out_file_ext = MP3_EXT;
			converted = true;

			fs::create_directory(temp_dst);

			cvt::convert_multiple(ffmpeg_exe_dir, file_list, temp_dst, out_file_ext);
		}

		split::split_multiple(ffmpeg_exe_dir, file_list, dst_dir, base_name, out_file_ext, segment_sec);

		if (converted) { // get rid of the converted files

			fs::remove_all(temp_dst);
		}
	}
	catch (std::exception & e)
	{
		std::cout << e.what();
		return false;
	}

	return true;
}

bool prompt_user() {

	bool prompt = true;
	bool quit = false;
	std::string selection;

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
			std::cout << "\n";
			break;

		default:
			std::cout << "\n";
			break;
		}

	}

	return !quit;
}

// display the inputs that the user chose/confirmed
void show_inputs() {
	std::cout << "          ffmpeg location: " << ffmpeg_exe_dir << "\n";
	std::cout << "           file extension: " << file_ext << "\n";
	std::cout << "            file location: " << src_dir << "\n";
	std::cout << "            new file name: " << base_name << "\n";
	std::cout << "        new file location: " << dst_dir << "\n";
	std::cout << "file segment length (sec): " << segment_sec << "\n";
}



// prompt the user for required inputs
void get_inputs() {

	std::string str;

	std::cout << "Enter ffmpeg location (" << ffmpeg_exe_dir << "):\n";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str) && fs::exists(str))
		ffmpeg_exe_dir = str;

	std::cout << "Enter file extension (" << file_ext <<"): ";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str))
		file_ext = str;

	std::cout << "Enter file location (" << src_dir << "):\n";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str) && fs::exists(str))
		src_dir = str;

	std::cout << "Enter new file name (" << base_name << "): ";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str))
		base_name = str;

	dst_dir = str::str_append_sub(src_dir, base_name);

	std::cout << "Enter new file location (" << dst_dir << "):\n";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str))
		dst_dir = str;

	std::cout << "Enter file segment length in seconds (" << segment_sec << "): ";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str) && str::str_is_unsigned(str))
		segment_sec = atoi(str.c_str());	

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