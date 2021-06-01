#include <type_traits>
#include <string>
#include <mutex>
#include <iostream>

#include "sort.hpp"
#include "utils_sort.hpp"

namespace sortec {
	using namespace utils::types;

	//==========================================================================================
	void sortBubbleST(const std::string& srcFilePath, const std::string& dstFilePath) {
		constexpr TYPE_SIZE OVEN_ELEMENTS_AMOUNT = 0x100;

		using ELEMENT_TYPE = TYPE_DWORD;
		constexpr auto ELEMENT_SIZE = sizeof(ELEMENT_TYPE);

		TYPE_DWORD oven[OVEN_ELEMENTS_AMOUNT];
		constexpr TYPE_SIZE OVEN_SIZE = OVEN_ELEMENTS_AMOUNT * ELEMENT_SIZE;


		std::ifstream streamSrcFile(srcFilePath, std::fstream::binary);
		std::ofstream streamOutputFile(dstFilePath, std::fstream::binary);

		//Get file size
		streamSrcFile.seekg(0, streamSrcFile.end);
		TYPE_FILESIZE fileSizePos = streamSrcFile.tellg();
		streamSrcFile.seekg(0, streamSrcFile.beg);

		const TYPE_SIZE CHUNKS_AMOUNT = fileSizePos / OVEN_SIZE;
		const TYPE_SIZE ELEMENTS_AMOUNT = fileSizePos / ELEMENT_SIZE;

		//Copy source file (in chunk mode) to be able to modify contents
		for (TYPE_SIZE i = 0; i != CHUNKS_AMOUNT; ++i) {
			streamSrcFile.read(reinterpret_cast<char*>(oven), OVEN_SIZE);
			streamOutputFile.write(reinterpret_cast<const char*>(oven), OVEN_SIZE);
		}
		const TYPE_SIZE REMAINING_CHUNK_SIZE = fileSizePos % OVEN_SIZE;
		if (0 != REMAINING_CHUNK_SIZE) {
			streamSrcFile.read(reinterpret_cast<char*>(oven), REMAINING_CHUNK_SIZE);
			streamOutputFile.write(reinterpret_cast<const char*>(oven), REMAINING_CHUNK_SIZE);
		}

		streamSrcFile.close();
		streamOutputFile.close();

		std::ifstream rstreamOutputFile(dstFilePath, std::fstream::binary | std::ios::out | std::ios::in);
		std::ofstream wstreamOutputFile(dstFilePath, std::fstream::binary | std::ios::out | std::ios::in);

		//Loop for each element
		for (TYPE_SIZE i{}; i != ELEMENTS_AMOUNT - 1; ++i) {
			rstreamOutputFile.seekg(rstreamOutputFile.beg + i * ELEMENT_SIZE);
			ELEMENT_TYPE curSrcElement;
			rstreamOutputFile.read(reinterpret_cast<char*>(&curSrcElement), ELEMENT_SIZE);

			ELEMENT_TYPE	curMinElement = curSrcElement;
			TYPE_SIZE		curMinElementIndex;
			//Search for min element
			for (auto j = i + 1; j != ELEMENTS_AMOUNT; ++j) {
				//Somewhere here chunk reading may be implemented
				rstreamOutputFile.seekg(rstreamOutputFile.beg + j * ELEMENT_SIZE);
				ELEMENT_TYPE	curElement;
				rstreamOutputFile.read(reinterpret_cast<char*>(&curElement), ELEMENT_SIZE);
				if (curElement < curMinElement) {
					curMinElement = curElement;
					curMinElementIndex = j;
				}
			}
			if (curSrcElement != curMinElement) {
				wstreamOutputFile.seekp(wstreamOutputFile.beg + i * ELEMENT_SIZE);
				wstreamOutputFile.write(reinterpret_cast<const char*>(&curMinElement), ELEMENT_SIZE);

				wstreamOutputFile.seekp(wstreamOutputFile.beg + curMinElementIndex * ELEMENT_SIZE);
				wstreamOutputFile.write(reinterpret_cast<const char*>(&curSrcElement), ELEMENT_SIZE);

				wstreamOutputFile.flush();
			}
		}
	}

	//==========================================================================================
	void sortFullInMem(const std::string& srcFilePath, const std::string& dstFilePath) {
		using ELEMENT_TYPE = TYPE_DWORD;

		std::ifstream streamSrcFile(srcFilePath, std::fstream::binary);

		//Get file size
		streamSrcFile.seekg(0, streamSrcFile.end);
		TYPE_FILESIZE fileSizePos = streamSrcFile.tellg();
		streamSrcFile.seekg(0, streamSrcFile.beg);

		const TYPE_SIZE ELEMENTS_AMOUNT = fileSizePos / sizeof(ELEMENT_TYPE);

		std::vector<ELEMENT_TYPE> oven(ELEMENTS_AMOUNT);

		streamSrcFile.read(reinterpret_cast<char*>(oven.data()), oven.size() * sizeof(ELEMENT_TYPE));
		streamSrcFile.close();

		std::sort(std::begin(oven), std::end(oven));

		std::ofstream streamOutputFile(dstFilePath, std::fstream::binary);
		streamOutputFile.write(reinterpret_cast<const char*>(oven.data()), oven.size() * sizeof(ELEMENT_TYPE));
		streamOutputFile.close();
	}

	//==========================================================================================
	std::string resultFolderPath;

	std::string srcFilePath;
	std::string dstFilePath;
	TYPE_FILESIZE fileSize;

	using ELEMENT_TYPE = TYPE_DWORD;
	constexpr std::size_t ELEMENT_SIZE = sizeof(ELEMENT_TYPE);

	const std::size_t		THREADS_AMOUNT = std::max<decltype(std::thread::hardware_concurrency())>(std::thread::hardware_concurrency(), 2);
	//const std::size_t		THREADS_AMOUNT = 2; //threads amount calced in runtime => only const
	constexpr std::size_t	MEMORY_SIZE1 = 128 * 1024 * 1024;
	const std::size_t		MEMORY_PER_THREAD = MEMORY_SIZE1 / THREADS_AMOUNT;
	const std::size_t		ELEMENTS_PER_THREAD = MEMORY_PER_THREAD / ELEMENT_SIZE;
	const std::size_t		MEMORY_SIZE = THREADS_AMOUNT * MEMORY_PER_THREAD;

	std::vector<ELEMENT_TYPE> oven = std::vector<ELEMENT_TYPE>(MEMORY_SIZE / ELEMENT_SIZE);

	std::size_t filesAmount;

	void workerFunc(const TYPE_SIZE par_workerIndex, const TYPE_SIZE roundsAmount) {
		std::ifstream ifStream(srcFilePath, std::fstream::binary);
		
		TYPE_FILESIZE curFilePos = par_workerIndex * MEMORY_PER_THREAD;

		for (std::size_t i{}; i != roundsAmount; ++i) {
			std::ofstream ofStream(resultFolderPath+ "0_" + std::to_string(par_workerIndex+ i * THREADS_AMOUNT), std::fstream::binary);

			ifStream.seekg(ifStream.beg + curFilePos);
			curFilePos += MEMORY_SIZE;

			ifStream.read(reinterpret_cast<char*>(oven.data()) + MEMORY_PER_THREAD * par_workerIndex, MEMORY_PER_THREAD);

			std::sort(oven.begin() + MEMORY_PER_THREAD * par_workerIndex / ELEMENT_SIZE, oven.begin() + (MEMORY_PER_THREAD * par_workerIndex + MEMORY_PER_THREAD) / ELEMENT_SIZE);

			ofStream.write(reinterpret_cast<const char*>(oven.data()) + MEMORY_PER_THREAD * par_workerIndex, MEMORY_PER_THREAD);

			ofStream.close();
		}
		ifStream.close();
	}

	void workerPartialFunc(const TYPE_SIZE par_workerIndex, const TYPE_SIZE roundsAmount) {
		std::ifstream ifStream(srcFilePath, std::fstream::binary);

		TYPE_FILESIZE curFilePos = par_workerIndex * MEMORY_PER_THREAD;

		for (std::size_t i{}; i != roundsAmount; ++i) {
			//std::ofstream ofStream(resultFolderPath + std::to_string(par_workerIndex) + '_' + std::to_string(i), std::fstream::binary);
			std::ofstream ofStream(resultFolderPath + "0_" + std::to_string(par_workerIndex + i * THREADS_AMOUNT), std::fstream::binary);

			ifStream.seekg(ifStream.beg + curFilePos);
			curFilePos += MEMORY_SIZE;

			ifStream.read(reinterpret_cast<char*>(oven.data()) + MEMORY_PER_THREAD * par_workerIndex, MEMORY_PER_THREAD);

			std::sort(oven.begin() + MEMORY_PER_THREAD * par_workerIndex / ELEMENT_SIZE, oven.begin() + (MEMORY_PER_THREAD * par_workerIndex + MEMORY_PER_THREAD) / ELEMENT_SIZE);

			ofStream.write(reinterpret_cast<const char*>(oven.data()) + MEMORY_PER_THREAD * par_workerIndex, MEMORY_PER_THREAD);

			ofStream.close();
		}
		
		//Process partial chunk
		std::ofstream ofStream(resultFolderPath + "0_" + std::to_string(par_workerIndex + roundsAmount * THREADS_AMOUNT), std::fstream::binary);
		ifStream.seekg(ifStream.beg + curFilePos);
		ifStream.read(reinterpret_cast<char*>(oven.data()) + MEMORY_PER_THREAD * par_workerIndex, MEMORY_PER_THREAD);
		const auto partialDataSize = ifStream.gcount();

		std::sort(oven.begin() + MEMORY_PER_THREAD * par_workerIndex / ELEMENT_SIZE, oven.begin() + (MEMORY_PER_THREAD * par_workerIndex + partialDataSize) / ELEMENT_SIZE);

		ofStream.write(reinterpret_cast<const char*>(oven.data()) + MEMORY_PER_THREAD * par_workerIndex, partialDataSize);

		ofStream.close();

		ifStream.close();
	}

	void sort(const std::string& par_srcFilePath, const std::string& par_dstFilePath) {
		//Get root folder path
		auto lastSlashPos = par_srcFilePath.find_last_of('\\');
		resultFolderPath = (std::string::npos == lastSlashPos) ? ("") : (par_srcFilePath.substr(0, lastSlashPos+1));
#ifdef D_LOGGING
		std::cout << "Root folder path: " << resultFolderPath << std::endl;

		std::cout << std::hex;

		std::cout	<< "MEMORY_SIZE=0x" << MEMORY_SIZE << std::endl
					<< "THREADS_AMOUNT=0x" << THREADS_AMOUNT << std::endl
					<< "MEMORY_PER_THREAD=0x" << MEMORY_PER_THREAD << std::endl
					<< "ELEMENTS_PER_THREAD=0x" << ELEMENTS_PER_THREAD << std::endl
					<< std::endl;
#endif
		srcFilePath = par_srcFilePath;
		dstFilePath = par_dstFilePath;

		
		std::ifstream streamSrcFile(srcFilePath, std::fstream::binary);
		streamSrcFile.seekg(0, streamSrcFile.end);
		fileSize = streamSrcFile.tellg();
		streamSrcFile.seekg(0, streamSrcFile.beg);
		streamSrcFile.close();
		/*
	//Copy source file (in chunk mode) to be able to modify contents

		std::ifstream streamSrcFile(srcFilePath, std::fstream::binary);
		std::ofstream streamOutputFile(dstFilePath, std::fstream::binary);

		//Get file size
		streamSrcFile.seekg(0, streamSrcFile.end);
		fileSize = streamSrcFile.tellg();
		streamSrcFile.seekg(0, streamSrcFile.beg);
		streamSrcFile.close();

		const TYPE_SIZE CHUNKS_AMOUNT = fileSize / MEMORY_SIZE;
		const TYPE_SIZE ELEMENTS_AMOUNT = fileSize / ELEMENT_SIZE;

		//Copy file
		for (TYPE_SIZE i = 0; i != CHUNKS_AMOUNT; ++i) {
			streamSrcFile.read(reinterpret_cast<char*>(oven.data()), MEMORY_SIZE);
			streamOutputFile.write(reinterpret_cast<const char*>(oven.data()), MEMORY_SIZE);
		}
		const TYPE_SIZE REMAINING_CHUNK_SIZE = fileSize % MEMORY_SIZE;
		if (0 != REMAINING_CHUNK_SIZE) {
			streamSrcFile.read(reinterpret_cast<char*>(oven.data()), REMAINING_CHUNK_SIZE);
			streamOutputFile.write(reinterpret_cast<const char*>(oven.data()), REMAINING_CHUNK_SIZE);
		}
		
		streamSrcFile.close();
		streamOutputFile.close();
		*/
		streamSrcFile.close();

		//Run workers
		const auto mainRoundsAmount = fileSize / MEMORY_SIZE;
		const auto moreRoundsAmount = (fileSize % MEMORY_SIZE) / MEMORY_PER_THREAD;
		const bool flagPartialBlockPresent = (fileSize % MEMORY_SIZE) % MEMORY_PER_THREAD != 0;

		//Set files amount
		filesAmount = mainRoundsAmount * THREADS_AMOUNT + moreRoundsAmount + flagPartialBlockPresent;
		

		std::vector<std::thread> vecWorkers;
		vecWorkers.reserve(THREADS_AMOUNT - 1);

		std::remove_const<decltype(THREADS_AMOUNT)>::type i{1};
		//main rounds amount + 1
		for (; i < moreRoundsAmount; ++i) {
			vecWorkers.emplace_back(workerFunc, i, mainRoundsAmount + 1);
		}

		//main rounds amount + partial
		if (moreRoundsAmount > 0) { //if not, current thread may handle partial chunk
			if (flagPartialBlockPresent) {
				vecWorkers.emplace_back(workerPartialFunc, i++, mainRoundsAmount);
			}
		}

		//main rounds amount only
		for (; i != THREADS_AMOUNT; ++i) {
			vecWorkers.emplace_back(workerFunc, i, mainRoundsAmount);
		}

		if (moreRoundsAmount > 0) {
			workerFunc(0, mainRoundsAmount + 1);
		} else if (flagPartialBlockPresent) {
			workerPartialFunc(0, mainRoundsAmount);
		} else {
			workerFunc(0, mainRoundsAmount);
		}

		for (auto&& e : vecWorkers) {
			e.join();
		}
	}

//====================================================================================================
	void mainSort(const std::string& par_srcFilePath, const std::string& par_dstFilePath) {
		//Divide input file on set of small sorted files
		sort(par_srcFilePath, par_dstFilePath);
#ifdef D_LOGGING
		std::cout << std::dec;
		std::cout << "Initial files amount=" << filesAmount << std::endl;
#endif
		decltype(filesAmount) curFileIndex;
		std::size_t lvlIndex=0;

		std::vector<std::thread> vecWorkers(THREADS_AMOUNT - 1); //std::forward_list workers;
		//vecWorkers.reserve(THREADS_AMOUNT - 1);

		while (filesAmount>3) {
			curFileIndex = 0;

			bool flagFilesAmountNotEven = filesAmount % 2;
#ifdef D_LOGGING
			std::cout << "LVL " << lvlIndex << ": current files amount=" << filesAmount << std::endl;
#endif
			filesAmount /= 2; //even if not even
			auto roundsAmount = filesAmount / THREADS_AMOUNT;
			auto remainingFilesAmount = filesAmount % THREADS_AMOUNT;
			for (decltype(roundsAmount) i = 0; i != roundsAmount;++i) {
				for (std::size_t j = 0; j != THREADS_AMOUNT - 1;++j) {
					//Rewrite by indexing workers
					std::vector<std::string> sortedArrayFilesPaths(2);
					sortedArrayFilesPaths[0] = resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((i * THREADS_AMOUNT + j) * 2);
					sortedArrayFilesPaths[1] = resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((i * THREADS_AMOUNT + j) * 2 + 1);
					vecWorkers[j] = std::thread(
						mergeMultipleSortedArrayFiles,
						sortedArrayFilesPaths,
						resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(i * THREADS_AMOUNT + j),
						reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * j, MEMORY_PER_THREAD
					);

					/*
					//TODO Remove deletion
					//Delete processed files
					std::remove(sortedArrayFilesPaths[0].c_str());
					std::remove(sortedArrayFilesPaths[1].c_str());
					*/
				}
				
				//Merge 2 last files in main thread
				mergeMultipleSortedArrayFiles({
					resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((i * THREADS_AMOUNT + (THREADS_AMOUNT - 1)) * 2),
					resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((i * THREADS_AMOUNT + (THREADS_AMOUNT - 1)) * 2 + 1)
					},
					resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(i * THREADS_AMOUNT + (THREADS_AMOUNT - 1)),
					reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * (THREADS_AMOUNT - 1), MEMORY_PER_THREAD
				);

				for (auto&& e: vecWorkers) {
					e.join();
				}
			}
#ifdef D_LOGGING
			std::cout << std::hex;

			//Process remaining files if any
			std::cout << "Process remaining files if any" << std::endl;
			std::cout << "\tremainingFilesAmount=" << remainingFilesAmount << std::endl;
#endif
			//NOTE If only 2-3 files remaining - process them in main thread
			const auto ENHANCED_MEMORY_PER_THREAD = MEMORY_SIZE / remainingFilesAmount;
			if (remainingFilesAmount!=0) {
				for (decltype(remainingFilesAmount) i = 0; i != remainingFilesAmount-1; ++i) {
					std::vector<std::string> sortedArrayFilesPaths(2);
					sortedArrayFilesPaths[0] = resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount * THREADS_AMOUNT + i) * 2);
					sortedArrayFilesPaths[1] = resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount * THREADS_AMOUNT + i) * 2 + 1);
#ifdef D_LOGGING
					std::cout << "\t (1) input file 0: " << sortedArrayFilesPaths[0] << std::endl;
					std::cout << "\t (1) input file 1: " << sortedArrayFilesPaths[1] << std::endl;
					std::cout << "\t (1) output file: " << (resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(roundsAmount * THREADS_AMOUNT + i)) << std::endl;
#endif
					vecWorkers[i] = std::thread(
						mergeMultipleSortedArrayFiles,
						sortedArrayFilesPaths,
						resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(roundsAmount * THREADS_AMOUNT + i),
						reinterpret_cast<TYPE_BYTE*>(oven.data()) + ENHANCED_MEMORY_PER_THREAD * i, ENHANCED_MEMORY_PER_THREAD
						//reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * i, MEMORY_PER_THREAD
					);
#ifdef D_LOGGING
					std::cout << "(1) Memory ptr: " << (std::size_t)(reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * i) << std::endl;
#endif
				}

				if (true == flagFilesAmountNotEven) {
#ifdef D_LOGGING
					std::cout << "\t (2) input file 0: " << (resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2)) << std::endl;
					std::cout << "\t (2) input file 1: " << (resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2 + 1)) << std::endl;
					std::cout << "\t (2) input file 2: " << (resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2 + 2)) << std::endl;
					std::cout << "\t (2) output file: " << (resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1))) << std::endl;
#endif
					//Merge 3 last files
					mergeMultipleSortedArrayFiles({
						resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount* THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2),
						resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount* THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2 + 1),
						resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount* THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2 + 2)
						},
						resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1)),
						reinterpret_cast<TYPE_BYTE*>(oven.data()) + ENHANCED_MEMORY_PER_THREAD * (remainingFilesAmount-1), ENHANCED_MEMORY_PER_THREAD
						//reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * (remainingFilesAmount-1), MEMORY_PER_THREAD
					);
#ifdef D_LOGGING
					std::cout << "(3) Memory ptr: " << (std::size_t)(reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * (remainingFilesAmount - 1)) << std::endl;
#endif
				} else {
#ifdef D_LOGGING
					std::cout << "\t (3) input file 0: " << (resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2)) << std::endl;
					std::cout << "\t (3) input file 1: " << (resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2 + 1)) << std::endl;
					std::cout << "\t (3) output file: " << (resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1))) << std::endl;
#endif
					//Merge 2 last files
					mergeMultipleSortedArrayFiles({
						resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount* THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2),
						resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string((roundsAmount* THREADS_AMOUNT + (remainingFilesAmount - 1)) * 2 + 1)
						},
						resultFolderPath + std::to_string(lvlIndex + 1) + "_" + std::to_string(roundsAmount * THREADS_AMOUNT + (remainingFilesAmount - 1)),
						reinterpret_cast<TYPE_BYTE*>(oven.data()) + ENHANCED_MEMORY_PER_THREAD * (remainingFilesAmount - 1), ENHANCED_MEMORY_PER_THREAD
						//reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * (remainingFilesAmount - 1), MEMORY_PER_THREAD
					);
#ifdef D_LOGGING
					std::cout << "(3) Memory ptr: " << (std::size_t)(reinterpret_cast<TYPE_BYTE*>(oven.data()) + MEMORY_PER_THREAD * (remainingFilesAmount - 1)) << std::endl;
#endif
				}

				for (decltype(remainingFilesAmount) i = 1; i < remainingFilesAmount; ++i) {
					vecWorkers[i-1].join();
				}
			}

			++lvlIndex;
		}

		//Merge last step files into outputFileName
		std::vector<std::string> sortedArrayFilesPaths(filesAmount);
		for (decltype(filesAmount) i = 0; i != filesAmount; ++i) {
			sortedArrayFilesPaths[i] = resultFolderPath + std::to_string(lvlIndex) + "_" + std::to_string(i);
		}
		mergeMultipleSortedArrayFiles(sortedArrayFilesPaths, par_dstFilePath,reinterpret_cast<TYPE_BYTE*>(oven.data()), MEMORY_SIZE);
	}
}