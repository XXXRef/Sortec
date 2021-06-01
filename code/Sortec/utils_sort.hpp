#pragma once

#include <vector>
#include <list>
#include <string>
#include <algorithm>

using TYPE_ELEMENT = std::uint32_t;
using TYPE_BYTE = std::uint8_t;
//using TYPE_SIZE = std::uint32_t;

struct ArrayBlob {
	TYPE_ELEMENT*	p;
	std::uint32_t		size;
};

void mergeMultipleSortedArrayFiles(const std::vector<std::string>& sortedArrayFilesPaths, const std::string& outputFilePath, TYPE_BYTE* const memory, const std::size_t MEM_SIZE);