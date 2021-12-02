#pragma once

// Windows file path
// Or add ffmpeg to PATH and change ffmpeg_exe_dir to "ffmpeg"
// Should work for Linux as well (not tested)
constexpr auto FFMPEG_EXE_DIR = R"(D:\ffmpeg\bin\)";

// default directory to look for audio files (for testing)
constexpr auto SRC_DIR_DEFAULT = R"(D:\ffmpeg\test_media\)";

constexpr auto FILE_NAME_BASE_DEFAULT = "split";

constexpr unsigned SEGMENT_SEC_DEFAULT = 600;

constexpr auto MP3_EXT = ".mp3";