#pragma once

#include <cstdlib>
#include <fstream>
#include <string>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <thread>

#include "utils.hpp"

namespace sortec{
	void sortBubbleST(const std::string& srcFilePath, const std::string& dstFilePath);

	void sortFullInMem(const std::string& srcFilePath, const std::string& dstFilePath);

	void sort(const std::string& par_srcFilePath, const std::string& par_dstFilePath);

	void mainSort(const std::string& par_srcFilePath, const std::string& par_dstFilePath);
}