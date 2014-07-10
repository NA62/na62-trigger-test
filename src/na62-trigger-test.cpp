//============================================================================
// Name        : NA62 online trigger algorithm test
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/detail/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <eventBuilding/SourceIDManager.h>
#include <utils/Utils.h>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <l0/MEP.h>

#include "options/MyOptions.h"

using namespace na62;

typedef struct {
	std::string binaryFile;
	int headerSourceID;
	int headerNumberOfReadOutBoards;
	int headerNumberOfEvents;
} HeaderData;

std::vector<boost::filesystem::path> getFilePaths(std::string directoryPath) {
	std::vector<boost::filesystem::path> files;

	boost::filesystem::directory_iterator endIterator, fileIterator =
			boost::filesystem::directory_iterator(
					boost::filesystem::path(directoryPath));
	for (; fileIterator != endIterator; fileIterator++) {
		files.insert(files.end(), fileIterator->path());
	}

	return files;
}

HeaderData getIntHeaderDataFromFile(boost::filesystem::path filePath) {
	std::ifstream file;
	std::string fileLine;
	std::vector<std::string> headerRawData(3);
	HeaderData headerData;

	file.open(filePath.string());

	getline(file, fileLine);
	headerData.binaryFile = fileLine;
	getline(file, fileLine);
	boost::algorithm::split(headerRawData, fileLine, boost::is_any_of(":"));

	headerData.headerSourceID = Utils::ToUInt(headerRawData[0]);
	headerData.headerNumberOfReadOutBoards = Utils::ToUInt(headerRawData[1]);
	headerData.headerNumberOfEvents = Utils::ToUInt(headerRawData[2]);

	file.close();

	return headerData;
}

std::vector<std::pair<int, int>> createSourceIDPairsVectorFromFiles(
		std::vector<int> sourceIDs,
		std::vector<boost::filesystem::path> filePaths) {
	std::vector<std::pair<int, int>> pairs;
	HeaderData headerData;
	int pairsCount = 0;

	for (int sourceID : sourceIDs) {
		for (auto path : filePaths) {
			if (boost::filesystem::is_regular(path.string())) {
				headerData = getIntHeaderDataFromFile(path);
				if (sourceID == headerData.headerSourceID) {
					pairs.insert(pairs.begin() + pairsCount++,
							std::make_pair(headerData.headerSourceID,
									headerData.headerNumberOfReadOutBoards));
				}
			}
		}
	}

	return pairs;
}

int main(int argc, char* argv[]) {
	/*
	 * Static Class initializations
	 */
	MyOptions::Load(argc, argv);


	std::vector<int> sourceIDs = Options::GetIntList(OPTION_ACTIVE_SOURCE_IDS);

	std::vector<boost::filesystem::path> files = getFilePaths(
			Options::GetString(OPTION_RAW_INPUT_DIR));

	std::vector<std::pair<int, int>> sourceIDPairsVector =
			createSourceIDPairsVectorFromFiles(sourceIDs, files);

	SourceIDManager::Initialize(SOURCE_ID_LAV, sourceIDPairsVector, { });


//	l0::MEP* mep = new l0::MEP(nullptr, 0, nullptr);

//	Event* e = new Event(0);
//	l0::MEP* mep = new l0::MEP();
//	L1TriggerProcessor t;

//	int i, eventCount = mep->getNumberOfEvents();
//	for (i = 0; i < eventCount; i++) {
//		if (e->addL0Event(mep->getEvent(i), 0)) {
//			t.compute(e);
//		}
//	}

	return 0;
}
