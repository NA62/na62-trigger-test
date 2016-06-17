//============================================================================
// Name        : NA62 online trigger algorithm test
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>

#include <boost/bind/placeholders.hpp>
#include <boost/filesystem/path.hpp>
#include <eventBuilding/Event.h>
#include <storage/BurstFileWriter.h>
#include <structs/Event.h>
#include <sys/types.h>
#include <utils/DataDumper.h>
#include <cstdbool>
#include <iostream>
#include <iterator>
#include <string>
#include <eventBuilding/SourceIDManager.h>
#include <l0/MEP.h>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include <l1/L1TriggerProcessor.h>
#include <l2/L2TriggerProcessor.h>
#include <common/HLTriggerManager.h>
#include <storage/EventSerializer.h>
#include <struct/HLTConfParams.h>

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

void writeBurstFile(test::EventBuilder& builder, uint burstID) {
	auto events = builder.getFinishedEvents();

	std::string fileName = Options::GetString(OPTION_RAW_FILE_NAME);
	if (!fileName.empty()) {
		std::string outputDir = Options::GetString(OPTION_OUTPUT_DIR);

		DataDumper::generateDirIfNotExists(outputDir);
		std::string filePath = DataDumper::generateFreeFilePath(fileName,
				outputDir);

		BurstFileWriter writer(filePath, fileName, events.size(),
				events[0]->getTimestamp(), 0, burstID);

		for (auto& event : events) {
			const EVENT_HDR* data = EventSerializer::SerializeEvent(event);
			writer.writeEvent(data);
			delete[] data;
			event->destroy();
		}
	}
}

int main(int argc, char* argv[]) {
	/*
	 * Unit tests (Will run all tests included in trigger-algorithms/UnitTests.h)
	 */
	boost::unit_test::unit_test_main(&init_function, argc, argv);

	L1TriggerProcessor::registerDownscalingAlgorithms();
	L1TriggerProcessor::registerReductionAlgorithms();

	/*
	 * Static Class initializations
	 */
	TriggerOptions::Load(argc, argv);
	MyOptions::Load(argc, argv);

//	HLTriggerManager::createXMLFile();

	HLTStruct HLTConfParams;
	HLTriggerManager::fillStructFromXMLFile(HLTConfParams);

	L1TriggerProcessor::initialize(HLTConfParams.l1);
	L2TriggerProcessor::initialize(HLTConfParams.l2);

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
	std::cout << "Reading " << headerFileExpressions.size()
			<< " header files: ";
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
	std::vector<std::pair<int, int>> sourceIDPairsVectorL1 = sourceIDPairsVector;
//	SourceIDManager::Initialize(Options::GetInt(OPTION_TS_SOURCEID),sourceIDPairsVector, { }, { }, -1);
	SourceIDManager::Initialize(Options::GetInt(OPTION_TS_SOURCEID),
			sourceIDPairsVector, sourceIDPairsVectorL1);

	EventSerializer::initialize();

	test::EventBuilder builder;

	for (int i = 0; i < 1; i++) {
		for (auto& header : headers) {
			std::function<void(l0::MEP_HDR*)> finishedMEPCallback = std::bind(
					&test::EventBuilder::buildMEP, &builder,
					std::placeholders::_1);
			FileReader::readDataFromFile(header, finishedMEPCallback);
		}
		writeBurstFile(builder, i);
	}

	LOG_INFO(
			"Number of L1 Input Events " << L1TriggerProcessor::GetL1InputStats());
	LOG_INFO(
			"Number of Accepted L1 Physics Triggers " << (uint) test::EventBuilder::GetL1AcceptedStats());
	LOG_INFO(
			"Number of isAllL1AlgoDisable Events " << (uint) test::EventBuilder::GetL1AllAlgoDisabledStats());
	LOG_INFO(
			"Number of L1 Bypassed Events " << (uint) test::EventBuilder::GetL1BypassedStats());
	LOG_INFO(
			"Number of Flagged L1 Algo processed " << (uint) test::EventBuilder::GetL1FlaggedAlgoProcessedStats());
	LOG_INFO(
			"Number of AutoPass/Flag Events " << (uint) test::EventBuilder::GetL1AutoPassFlagStats());
	std::cout << "#################################" << std::endl;
	std::cout << "Finished processing all data without any fatal errors!"
			<< std::endl;

	return 0;
}
