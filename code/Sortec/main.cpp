#include <chrono>
#include <iostream>

#include "validate_sort.hpp"
#include "utils_sort.hpp"
#include "generation.hpp"

#include "sort.hpp"

using namespace utils::types;

void main_validate(const std::string& par_filePath) {
	auto startTime = std::chrono::high_resolution_clock::now();
	auto validationResult = validateFileSortInMem(par_filePath);
	auto finishTime = std::chrono::high_resolution_clock::now();
	std::printf("Validation result: %d\n", validationResult);
	std::printf("\tProcessing time: %f secs\n", std::chrono::duration<double>(finishTime - startTime).count());
}

void main_generate(const std::string& par_filePath, TYPE_SIZE elementsAmount) {
	produceRandomFile(par_filePath, elementsAmount);
}

void main_construct(const std::string& par_filePath, TYPE_SIZE elementsAmount) {
	produceSortedFile(par_filePath, elementsAmount);
}

void main_sort(const std::string& par_filePath, const std::string& par_outFilePath) {
	using namespace sortec;

	auto startTime = std::chrono::high_resolution_clock::now();
	//sortBubbleST(par_filePath, par_outFilePath);
	//sortFullInMem(par_filePath, par_outFilePath);
	sortec::mainSort(par_filePath, par_outFilePath);
	auto finishTime = std::chrono::high_resolution_clock::now();
	std::printf("Sorting finished\n");
	std::printf("\tProcessing time: %f secs\n", std::chrono::duration<double>(finishTime - startTime).count());
}


#ifdef CONFIGURATION_TESTING
#include "utils_sort.hpp"

int main(int argc, char* argv[]) {

	std::vector<TYPE_ELEMENT> v1 = { 1,2,3,4,4,5,5,7,8};
	std::vector<TYPE_ELEMENT> v2 = { 1,1,26,32};
	std::vector<TYPE_ELEMENT> v3= {2,2,3,4,7,8,45,78};

	std::vector<TYPE_ELEMENT> vOutput(100);

	std::vector<ArrayBlob> collectionsContainer{
		{&v1[0],(std::uint32_t)(v1.size())},
		{&v2[0],(std::uint32_t)(v2.size())},
		{&v3[0],(std::uint32_t)(v3.size())}
	};

	mergeMultipleSortedCollections(collectionsContainer, { &vOutput[0],(std::uint32_t)(vOutput.size()) });

	for (auto&& e : vOutput) {
		std::cout << e << " ";
	}
	std::cout << std::endl;

	return EXIT_SUCCESS;
}
#else

/*
constexpr std::size_t	MEMORY_SIZE1 = 128 * 1024 * 1024;
TYPE_BYTE oven[MEMORY_SIZE1];
*/

int main(int argc,char* argv[]) {
	if (argc<3) {
		//Sort "infile" file in same folder
#ifdef D_LOGGING
		std::printf("Sorting file input -> output\n");
#endif
		const std::string& inputFilePath = "input";
		const std::string& outputFilePath = "output";
		main_sort(inputFilePath, outputFilePath);
	}

	auto cmd = argv[1];
	auto filePath = argv[2];

	//Generate file with random contents
	if (!std::strcmp("gen", cmd)) {
		TYPE_SIZE elementsAmount = std::atoi(argv[3]);
		main_generate(filePath, elementsAmount);
	}
	//Generate sorted file
	else if (!std::strcmp("construct", cmd)) {
		TYPE_SIZE elementsAmount = std::atoi(argv[3]);
		main_construct(filePath, elementsAmount);
	}
	//Verify file contents is sorted
	else if (!std::strcmp("validate", cmd)) {
		main_validate(filePath);
	}
	//Sort file, save results in output file
	else if(!std::strcmp("sort", cmd)){
		auto outFilePath = argv[3];
		main_sort(filePath, outFilePath);
	}
	/*
	//Merge files
	else if (!std::strcmp("merge", cmd)) {
		auto outFilePath = argv[2];
		mergeMultipleSortedArrayFiles({
				"C:/sortec_test/0_58",
				"C:/sortec_test/0_59"
			}, outFilePath,oven, MEMORY_SIZE1);
	}
	*/
	else {
		std::printf("[ERROR] Invalid command\n");
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

#endif