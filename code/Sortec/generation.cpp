#include "generation.hpp"

using namespace utils::types;

//==========================================================================================
void produceSortedFile(const std::string& filePath, std::size_t elementsAmount) {
	std::srand(std::time(nullptr));

	std::ofstream ifStream(filePath, std::fstream::binary);

	constexpr std::size_t MAX_REPEAT_AMOUNT = 4;

	for (TYPE_DWORD i = 0; i != elementsAmount; ++i) {
		auto repeatAmount = std::rand() % MAX_REPEAT_AMOUNT;
		for (TYPE_DWORD j = 0; j != repeatAmount; ++j) {
			ifStream.write(reinterpret_cast<const char*>(&i), sizeof(i));
		}
	}

	ifStream.close();
}

//==========================================================================================
void produceRandomFile(const std::string& filePath, std::size_t elementsAmount) {
	std::srand(std::time(nullptr));

	std::ofstream ifStream(filePath, std::fstream::binary);

	for (TYPE_DWORD i = 0; i != elementsAmount; ++i) {
		TYPE_DWORD curRandElement = std::rand() * 0x12345678; //Or simply gen random bytes
		ifStream.write(reinterpret_cast<const char*>(&curRandElement), sizeof(curRandElement));
	}

	ifStream.close();
}