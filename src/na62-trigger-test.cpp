//============================================================================
// Name        : NA62 online trigger algorithm test
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>

#include <boost/bind/placeholders.hpp>
#include <boost/filesystem/path.hpp>
#include <eventBuilding/SourceIDManager.h>
#include <l0/MEP.h>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include <l1/L1TriggerProcessor.h>
#include <l2/L2TriggerProcessor.h>
#include <eventBuilding/EventSerializer.h>

#include <options/TriggerOptions.h>
#include <UnitTests.h>

#include "EventBuilder.h"
#include "FileReader.h"
#include "options/MyOptions.h"

using namespace na62;
using namespace na62::test;

bool init_function() {
	return true;
}

int main(int argc, char* argv[]) {
	/*
	 * Unit tests (Will run all tests included in trigger-algorithms/UnitTests.h)
	 */
	boost::unit_test::unit_test_main(&init_function, argc, argv);

	/*
	 * Static Class initializations
	 */
	TriggerOptions::Load(argc, argv);
	MyOptions::Load(argc, argv);

	L1TriggerProcessor::initialize(
			TriggerOptions::GetDouble(OPTION_L1_BYPASS_PROBABILITY),
			TriggerOptions::GetInt(OPTION_L1_BYPASS_TRIGGER_WORD));
	L2TriggerProcessor::initialize(
			TriggerOptions::GetDouble(OPTION_L2_BYPASS_PROBABILITY),
			TriggerOptions::GetInt(OPTION_L2_BYPASS_TRIGGER_WORD),
			TriggerOptions::GetInt(OPTION_L1_BYPASS_TRIGGER_WORD));

	std::vector<int> sourceIDs = MyOptions::GetIntList(
	OPTION_ACTIVE_SOURCE_IDS);

	/*
	 * Extracting input header files from argument list
	 */
	std::vector<std::string> headerFileExpressions;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			headerFileExpressions.push_back(std::string(argv[i]));
		}
	}

	if (headerFileExpressions.empty()) {
		std::cerr
				<< "No input header files provided. Please use something like following:\n\t"
				<< argv[0] << " files/*.txt" << std::endl;
		return 1;
	}

	/*
	 * Read all header files
	 */
	std::cout << "Reading " << headerFileExpressions.size() << " header files: ";
	for (auto& headerFile : headerFileExpressions) {
		std::cout << headerFile;
		if (&headerFile != &*--headerFileExpressions.end())
			std::cout << ", ";
	}
	std::cout << std::endl;

	std::vector<HeaderData> headers = FileReader::getActiveHeaderData(sourceIDs,
			headerFileExpressions);


	if (headers.empty()) {
		std::cout << "Did not find any header file!" << std::endl;
		return 0;
	}

	std::cout << "Found " << headers.size() << " header files:" << std::endl;

	/*
	 * Initialize the SourceIDManager with all found sourceIDs
	 */
	std::vector<std::pair<int, int>> sourceIDPairsVector;
	for (HeaderData header : headers) {
		std::cout << "Found header file for " << header.binaryFile << std::endl;
		sourceIDPairsVector.push_back(
				std::move(
						std::make_pair(header.sourceID,
								header.numberOfReadOutBoards)));
	}
	SourceIDManager::Initialize(Options::GetInt(OPTION_TS_SOURCEID),
			sourceIDPairsVector, { }, { }, -1);

	EventSerializer::initialize();

	test::EventBuilder builder;

	for (auto& header : headers) {
		std::function<void(l0::MEP_HDR*)> finishedMEPCallback = std::bind(
				&test::EventBuilder::buildMEP, &builder, std::placeholders::_1);
		FileReader::readDataFromFile(header, finishedMEPCallback);
	}

	return 0;
}
