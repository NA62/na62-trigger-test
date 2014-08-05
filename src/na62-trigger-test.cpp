//============================================================================
// Name        : NA62 online trigger algorithm test
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================

#include <boost/bind/placeholders.hpp>
#include <boost/filesystem/path.hpp>
#include <eventBuilding/SourceIDManager.h>
#include <l0/MEP.h>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "EventBuilder.h"
#include "FileReader.h"
#include "options/MyOptions.h"

using namespace na62;
using namespace na62::test;

int main(int argc, char* argv[]) {
	/*
	 * Static Class initializations
	 */
	MyOptions::Load(argc, argv);

	std::vector<int> sourceIDs = MyOptions::GetIntList(
	OPTION_ACTIVE_SOURCE_IDS);

	std::vector<boost::filesystem::path> headerFiles =
			FileReader::getHeaderFiles(
					MyOptions::GetString(OPTION_RAW_INPUT_DIR));

	std::vector<HeaderData> headers = FileReader::getActiveHeaderData(sourceIDs,
			headerFiles);

	std::vector<std::pair<int, int>> sourceIDPairsVector;
	for (HeaderData header : headers) {
		sourceIDPairsVector.push_back(
				std::move(
						std::make_pair(header.sourceID,
								header.numberOfReadOutBoards)));
	}

	SourceIDManager::Initialize(SOURCE_ID_LAV, sourceIDPairsVector, { }, { });

	test::EventBuilder builder;

	std::vector<l0::MEP*> meps;
	for (auto header : headers) {
		std::function<void(l0::MEP_HDR*)> finishedMEPCallback = std::bind(
				&test::EventBuilder::buildMEP, &builder, std::placeholders::_1);
		meps = FileReader::getDataFromFile(header, finishedMEPCallback);
	}

	return 0;
}
