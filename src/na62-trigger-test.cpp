//============================================================================
// Name        : NA62 online trigger algorithm test
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/included/unit_test.hpp>

#include <boost/bind/placeholders.hpp>
#include <boost/filesystem/path.hpp>
#include <eventBuilding/SourceIDManager.h>
#include <l0/MEP.h>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include <options/TriggerOptions.h>
#include <UnitTests.h>

#include "EventBuilder.h"
#include "FileReader.h"
#include "options/MyOptions.h"

using namespace na62;
using namespace na62::test;

bool init_function() {
	// But, you CAN'T use testing tools here
	return true;
}

int main(int argc, char* argv[]) {
	boost::unit_test::unit_test_main(&init_function, argc, argv);
	/*
	 * Static Class initializations
	 */
	TriggerOptions::Load(argc, argv);
	MyOptions::Load(argc, argv);

	std::vector<int> sourceIDs = MyOptions::GetIntList(
	OPTION_ACTIVE_SOURCE_IDS);

	/*
	 * Read all header files
	 */
	std::vector<boost::filesystem::path> headerFiles =
			FileReader::getHeaderFiles(
					MyOptions::GetString(OPTION_RAW_INPUT_DIR));

	std::cout << "Found " << headerFiles.size() << " header files"<< std::endl;

	std::vector<HeaderData> headers = FileReader::getActiveHeaderData(sourceIDs,
			headerFiles);
	std::cout << "Read " << headers.size() << " data files" << std::endl;

	/*
	 * Initialize the SourceIDManager with all found sourceIDs
	 */
	std::vector<std::pair<int, int>> sourceIDPairsVector;
	for (HeaderData header : headers) {
		std::cout << "Reading header file for " << header.binaryFile
				<< std::endl;
		sourceIDPairsVector.push_back(
				std::move(
						std::make_pair(header.sourceID,
								header.numberOfReadOutBoards)));
	}
	SourceIDManager::Initialize(Options::GetInt(OPTION_TS_SOURCEID),
			sourceIDPairsVector, { }, { }, -1);

	test::EventBuilder builder;

	std::vector<l0::MEP*> meps;
	for (auto& header : headers) {
		std::function<void(l0::MEP_HDR*)> finishedMEPCallback = std::bind(
				&test::EventBuilder::buildMEP, &builder, std::placeholders::_1);
		meps = FileReader::getDataFromFile(header, finishedMEPCallback);
	}

	return 0;
}
