#include "utils_sort.hpp"

#include <vector>
#include <thread>
#include <memory>
#include <list>

#include <set>
#include <fstream>
#include <algorithm>

#include <cassert>

using TYPE_BYTE = std::uint8_t;

/*
template<class TCollectionsContainer, class TOutputCollection> void mergeMultipleSortedCollections(
	const TCollectionsContainer& par_collectionsContainer, TOutputCollection& par_outputCollection) {
	std::list<decltype((*(TCollectionsContainer.begin())).cbegin())> collectionsProcessingInfo;
	std::assert(collectionsProcessingInfo.size() > []{
		decltype((*(TCollectionsContainer.begin())).size()) size=0;
		for (auto&& e : par_collectionsContainer) {
			size += e.size();
		}
		return size;
		}());

	for (auto&& e : par_collectionsContainer) { collectionsProcessingInfo.push_back(e.cbegin()); }

}
*/

void mergeMultipleSortedCollections(const std::vector<ArrayBlob>& par_collectionsContainer, const ArrayBlob& par_outputCollection) {
	struct CollectionsProcessingBlobInfo {
		const ArrayBlob&		collection;
		std::uint32_t curElementIndex;
	};
	std::list<CollectionsProcessingBlobInfo> collectionsProcessingInfo;
	for (const auto& e: par_collectionsContainer) { collectionsProcessingInfo.push_back({e,0}); } //WARNING par_collectionsContainer elements order being reversed here (if forward_list used)

	CollectionsProcessingBlobInfo outputProcessingBlobInfo{ par_outputCollection,0 };

	auto Comparator = std::less<TYPE_ELEMENT>();
	while (true) {
		//Initialize cur element
		auto iterCurExtremumCollection = collectionsProcessingInfo.begin();
		auto curExtremumElement = (iterCurExtremumCollection->collection.p)[iterCurExtremumCollection->curElementIndex];
		
		//TODO caching current elements for every collection (reasonable for large amount of collections)
		
		//Get actual extremum element
		for (auto i = std::next(collectionsProcessingInfo.begin(),1); i != collectionsProcessingInfo.end();++i) {
			if (Comparator( ((*i).collection.p)[(*i).curElementIndex], curExtremumElement) ) {
				curExtremumElement = ((*i).collection.p)[(*i).curElementIndex];
				iterCurExtremumCollection = i;
			}
		}
		//Add extremum element into output collection
		outputProcessingBlobInfo.collection.p[outputProcessingBlobInfo.curElementIndex++] = curExtremumElement;

		//Advance appropriate source collection
		if ((*iterCurExtremumCollection).collection.size == (++(*iterCurExtremumCollection).curElementIndex)) {
			collectionsProcessingInfo.erase(iterCurExtremumCollection); 

			if (collectionsProcessingInfo.empty()) { break; }
		}
	}
}

//==========================================================================================
/*
constexpr std::size_t MEM_SIZE = 1001;
TYPE_BYTE memory[MEM_SIZE]; //memory per thread
*/

void mergeMultipleSortedArrayFiles(const std::vector<std::string>& sortedArrayFilesPaths, const std::string& outputFilePath,
	TYPE_BYTE* const memory, const std::size_t MEM_SIZE) {
	/*
	constexpr std::size_t MEMORY_PER_FILE = 0x800000;
	using ELEMENT_TYPE = std::uint32_t;
	constexpr std::size_t ELEMENT_SIZE = sizeof(ELEMENT_TYPE);
	constexpr std::size_t	ELEMENTS_PER_THREAD = MEMORY_PER_FILE / ELEMENT_SIZE;
	*/

	const auto FILES_AMOUNT = sortedArrayFilesPaths.size();

	const std::size_t MEMORY_PER_FILE = (MEM_SIZE / 2 / FILES_AMOUNT / sizeof(TYPE_ELEMENT)) * sizeof(TYPE_ELEMENT);//MEMORY_PER_FILE actually //MEMORY_PER_FILE aligned to size of TYPE_ELEMENT
	TYPE_BYTE* const pOutputDataBegin = memory + FILES_AMOUNT * MEMORY_PER_FILE;
	TYPE_BYTE* const pOutputDataEnd = pOutputDataBegin + ((MEM_SIZE - FILES_AMOUNT * MEMORY_PER_FILE) / sizeof(TYPE_ELEMENT)) * sizeof(TYPE_ELEMENT);
	const std::size_t outputDataSize = pOutputDataEnd - pOutputDataBegin;

	//std::assert(FILES_AMOUNT != 0 && "THREADS AMOUNT==0");
	
	//Loading arrays from files into memory
	struct FileMemInfo {
		std::size_t fileID; //also shows where its mem block
		std::ifstream ifStream;
		std::size_t curIndex;
		std::size_t size; //size in elements
	};

	std::set<std::unique_ptr<FileMemInfo>> processingInfo; //TIP std::set provides only const iterators -> use std::unique_ptr
	
	// Initialization
	TYPE_BYTE* pCurOutputData = pOutputDataBegin;
	for (std::size_t i = 0; i != sortedArrayFilesPaths.size();++i) {
		//Get input file size
		std::ifstream streamSrcFile(sortedArrayFilesPaths[i], std::fstream::binary);
		streamSrcFile.seekg(0, streamSrcFile.end);
		auto fileSize = streamSrcFile.tellg();
		streamSrcFile.seekg(0, streamSrcFile.beg);

		//Create record
		auto addedRecordInfo = processingInfo.insert(std::unique_ptr<FileMemInfo>(new FileMemInfo{ i,std::move(streamSrcFile),0,0 }));
		//Initial block loading
		auto& curFileMemInfo = *addedRecordInfo.first;
		curFileMemInfo->ifStream.read(reinterpret_cast<char*>(memory) + MEMORY_PER_FILE * curFileMemInfo->fileID, MEMORY_PER_FILE);
		curFileMemInfo->size = curFileMemInfo->ifStream.gcount() / sizeof(TYPE_ELEMENT);
	}

	std::ofstream hOutputFile(outputFilePath, std::fstream::binary);

	//Sorting
	while (true) {
		//auto pCurMinFileMemInfo = const_cast<FileMemInfo*>(&(*processingInfo.begin()));
		auto pCurMinFileMemInfo = processingInfo.begin();
		TYPE_ELEMENT curMinElement = reinterpret_cast<TYPE_ELEMENT*>(memory + (*pCurMinFileMemInfo)->fileID * MEMORY_PER_FILE)[(*pCurMinFileMemInfo)->curIndex];
		
		for (auto it = ++processingInfo.begin(); it != processingInfo.end();++it) {
			auto& curFileMemInfo = *it;
			TYPE_ELEMENT curElement = reinterpret_cast<TYPE_ELEMENT*>(memory+curFileMemInfo->fileID * MEMORY_PER_FILE)[curFileMemInfo->curIndex];
			if (curElement < curMinElement) { //TODO abstract comparison operation and entities' names
				curMinElement = curElement;
				pCurMinFileMemInfo = it;
			}
		}

		*((reinterpret_cast<TYPE_ELEMENT*>(pCurOutputData))) = curMinElement;
		pCurOutputData+=sizeof(TYPE_ELEMENT);

		if (pOutputDataEnd==pCurOutputData) {
			//Output block is filled up - output into file and refresh state
			hOutputFile.write(reinterpret_cast<const char*>(pOutputDataBegin), outputDataSize);
			pCurOutputData = pOutputDataBegin;
		}

		if (++(*pCurMinFileMemInfo)->curIndex==(*pCurMinFileMemInfo)->size) {
			//Load more || remove record ()
			//If no more records -> break
			(*pCurMinFileMemInfo)->ifStream.read(reinterpret_cast<char*>(memory) + MEMORY_PER_FILE * (*pCurMinFileMemInfo)->fileID, MEMORY_PER_FILE);
			(*pCurMinFileMemInfo)->size = (*pCurMinFileMemInfo)->ifStream.gcount() / sizeof(TYPE_ELEMENT);
			//Check whether file ended
			if (0==(*pCurMinFileMemInfo)->size) {
				(*pCurMinFileMemInfo)->ifStream.close();
				
				processingInfo.erase(pCurMinFileMemInfo);
				if (processingInfo.empty()) {
					//Output remaining output block into file
					if (pOutputDataBegin != pCurOutputData) {
						hOutputFile.write(reinterpret_cast<const char*>(pOutputDataBegin), pCurOutputData - pOutputDataBegin);
					}

					//Job is done - break the loop
					break;
				}
			}else{
				(*pCurMinFileMemInfo)->curIndex = 0;
			}
		}
	}
	hOutputFile.close();
}