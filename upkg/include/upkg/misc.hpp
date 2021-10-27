#pragma once

#include <tuple>
#include <string>

#include "zip.h"

namespace util {
	std::tuple<bool, std::string> compress_gz(const char* inFile, const char* outFile);
	std::tuple<bool, std::string> compress_zip(const char* inFile, const char* outFile);
}
