#include "app_config.hpp"
#include "str_helper.hpp"
#include "split.hpp"
#include "convert.hpp"

#include<exception>
#include<iostream>
#include<string>
#include<vector>

/*
LICENSE INFO:
Free to use for anyone who finds it.
*/

namespace fs = std::filesystem; // c++17

namespace str = str_helper;
namespace cvt = convert;



std::vector<fs::path> get_files_of_type(fs::path const& src_dir, std::string& extension);
void get_inputs();
void show_inputs();
bool prompt_user();
bool split_files();

//====== DEFAULTS USER INPUTS ====================

static std::string file_ext_in = MP3_EXT;
static std::string src_dir_in = fs::current_path().string();
static std::string base_name_in = FILE_NAME_BASE_DEFAULT;
static std::string dst_dir_in;
static unsigned segment_sec_in = SEGMENT_SEC_DEFAULT;
static std::string ffmpeg_exe_dir_in = FFMPEG_EXE_DIR;


int main() 
{
	if (!prompt_user())
		return EXIT_SUCCESS;

	if (!split_files())
		return EXIT_FAILURE;
}

//===================================


bool split_files() 
{
	auto src_dir = fs::path(src_dir_in);
	auto const ffmpeg_dir = fs::path(ffmpeg_exe_dir_in);

	if (segment_sec_in == 0
		|| !fs::exists(src_dir) 
		|| !fs::is_directory(src_dir)
		|| !fs::exists(ffmpeg_dir)
		|| !fs::is_directory(ffmpeg_dir)
		)
		return false;

	auto const dst_dir = fs::path(dst_dir_in);

	std::string out_file_ext = MP3_EXT; // try to convert everything to mp3

	bool to_convert = file_ext_in != out_file_ext;

	try
	{		
		fs::create_directories(dst_dir_in);

		if (to_convert)
		{
			auto file_list = get_files_of_type(src_dir, file_ext_in);
			auto temp_dst = dst_dir / "convert_temp";
			fs::create_directory(temp_dst);
			cvt::convert_multiple(ffmpeg_dir, file_list, temp_dst, out_file_ext);

			src_dir = temp_dst;
		}
		
		auto file_list = get_files_of_type(src_dir, out_file_ext);
		split::split_multiple(ffmpeg_dir, file_list, dst_dir, base_name_in, out_file_ext, segment_sec_in);

		if (to_convert) { // get rid of the converted files

			fs::remove_all(src_dir);
		}
	}
	catch (std::exception & e)
	{
		std::cout << e.what();
		return false;
	}

	return true;
}


bool prompt_user() 
{
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
void show_inputs() 
{
	std::cout << "          ffmpeg location: " << ffmpeg_exe_dir_in << "\n";
	std::cout << "           file extension: " << file_ext_in << "\n";
	std::cout << "            file location: " << src_dir_in << "\n";
	std::cout << "            new file name: " << base_name_in << "\n";
	std::cout << "        new file location: " << dst_dir_in << "\n";
	std::cout << "file segment length (sec): " << segment_sec_in << "\n";
}



// prompt the user for required inputs
void get_inputs() 
{
	std::string str;

	std::cout << "Enter ffmpeg location (" << ffmpeg_exe_dir_in << "):\n";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str) && fs::exists(str))
		ffmpeg_exe_dir_in = str;

	std::cout << "Enter file extension (" << file_ext_in <<"): ";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str))
		file_ext_in = str;

	std::cout << "Enter file location (" << src_dir_in << "):\n";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str) && fs::exists(str))
		src_dir_in = str;

	std::cout << "Enter new file name (" << base_name_in << "): ";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str))
		base_name_in = str;

	dst_dir_in = (fs::path(src_dir_in) / base_name_in).string();

	std::cout << "Enter new file location (" << dst_dir_in << "):\n";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str))
		dst_dir_in = str;

	std::cout << "Enter file segment length in seconds (" << segment_sec_in << "): ";
	std::getline(std::cin, str);
	if (!str::str_is_blank(str) && str::str_is_unsigned(str))
		segment_sec_in = atoi(str.c_str());	

}



// returns all files in a directory with a given extension
std::vector<fs::path> get_files_of_type(fs::path const& src_dir, std::string& extension) 
{
	// extenstion must begin with '.'
	if (extension[0] != '.')
		extension = "." + extension;

	std::vector<fs::path> file_list;

	if (!fs::exists(src_dir) || !fs::is_directory(src_dir))
		return file_list;

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