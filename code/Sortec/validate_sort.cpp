#include "validate_sort.hpp"

using namespace utils::types;

//==========================================================================================
bool validateFileSort(const std::string& filePath) {
	std::ifstream ifStream(filePath, std::fstream::binary);

	constexpr TYPE_SIZE CHUNK_ELEMENT_AMOUNT = 0x100;
	constexpr TYPE_SIZE CHUNK_SIZE = CHUNK_ELEMENT_AMOUNT * sizeof(TYPE_DWORD);
	TYPE_DWORD oven[CHUNK_ELEMENT_AMOUNT];

	std::streampos pos = CHUNK_SIZE;

	//Get file size
	ifStream.seekg(0, ifStream.end);
	TYPE_FILESIZE fileSizePos = ifStream.tellg();
	ifStream.seekg(0, ifStream.beg);

	const auto amountOfFullChunks = fileSizePos / CHUNK_SIZE;

	TYPE_DWORD trailingElement = 0; //Set minimum element value 

	if (0 != amountOfFullChunks) {
		//Process first chunk without involving trailing element
		//Read chunk
		ifStream.read(reinterpret_cast<char*>(oven), CHUNK_SIZE);
		//Check chunk sorted
		if (!checkIfArraySorted(oven, CHUNK_ELEMENT_AMOUNT)) {
			return false;
		}
		trailingElement = oven[CHUNK_ELEMENT_AMOUNT - 1];

		//Process full chunks
		for (decltype(fileSizePos / CHUNK_SIZE) i = 1; i != amountOfFullChunks; ++i) {
			//Read chunk
			ifStream.read(reinterpret_cast<char*>(oven), CHUNK_SIZE);
			//Check trailing element
			if (std::greater<TYPE_DWORD>()(trailingElement, oven[0])) {
				return false;
			}
			//Check chunk sorted
			if (!checkIfArraySorted(oven, CHUNK_ELEMENT_AMOUNT)) {
				return false;
			}
			trailingElement = oven[CHUNK_ELEMENT_AMOUNT - 1];
		}
	}

	//Process remaining block if it exists
	//Read chunk
	auto remainingChunkSize = fileSizePos % CHUNK_SIZE;
	if (0 == remainingChunkSize) {
		return true;
	}
	ifStream.read(reinterpret_cast<char*>(oven), remainingChunkSize);
	//Check trailing element
	if (std::greater<TYPE_DWORD>()(trailingElement, oven[0])) {
		return false;
	}
	//Check chunk sorted
	if (!checkIfArraySorted(oven, remainingChunkSize / sizeof(TYPE_DWORD))) {
		return false;
	}

	return true;
}

//==========================================================================================
bool validateFileSortInMem(const std::string& filePath) {

	std::cout << "validateFileSortInMem" << std::endl;

	std::ifstream ifStream(filePath, std::fstream::binary);

	TYPE_DWORD* pOven;

	//Get file size
	ifStream.seekg(0, ifStream.end);
	TYPE_FILESIZE fileSizePos = ifStream.tellg();
	ifStream.seekg(0, ifStream.beg);

	pOven = new TYPE_DWORD[fileSizePos/sizeof(TYPE_DWORD)];

	ifStream.read(reinterpret_cast<char*>(pOven), fileSizePos);

	ifStream.close();

	//Check chunk sorted
	auto result=checkIfArraySorted(pOven, fileSizePos / sizeof(TYPE_DWORD)) ? (true) : (false);

	delete[] pOven;

	return result;
}

//==========================================================================================
/*
bool validateFileSortMT(const std::string& filePath) {
	std::ifstream ifStream(filePath, std::fstream::binary);

	const auto THREADS_AMOUNT = std::max<decltype(std::thread::hardware_concurrency())>(std::thread::hardware_concurrency(),1);

	return true;
}
*/