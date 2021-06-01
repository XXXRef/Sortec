#pragma once

#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

#include "utils.hpp"

void produceSortedFile(const std::string& filePath, std::size_t elementsAmount);

void produceRandomFile(const std::string& filePath, std::size_t elementsAmount);