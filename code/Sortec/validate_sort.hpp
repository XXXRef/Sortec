#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <cstdint>

#include "utils.hpp"

//==========================================================================================
template<class T, class CmpF=std::less<T>> inline bool checkIfArraySorted(const T* pArray, std::size_t elemAmount, CmpF cmpFunction = std::less<T>()) {
	std::cout << std::hex;
	std::cout << "checkIfArraySorted " << " pArray=0x" << pArray << " elemAmount=0x" << elemAmount << std::endl;
	
	if (elemAmount < 2) {
		return true;
	}

	for (std::size_t i = 0; i != elemAmount - 1; ++i) {
		if (cmpFunction(pArray[i + 1], pArray[i])) {
			std::cout << "\tCmp function failed. Val1=0x" << pArray[i] << " Val2=0x" << pArray[i + 1] << " I=0x" << i << std::endl;

			return false;
		}
	}

	return true;
}

//==========================================================================================
bool validateFileSort(const std::string& filePath);

//==========================================================================================
bool validateFileSortInMem(const std::string& filePath);

//==========================================================================================
bool validateFileSortMT(const std::string& filePath);