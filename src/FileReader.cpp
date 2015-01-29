/*
 * FileReader.cpp
 *
 *  Created on: Jul 22, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "FileReader.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/detail/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <l0/MEP.h>
#include <l0/MEPFragment.h>
#include <utils/Utils.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <options/Logging.h>

#include "options/MyOptions.h"

namespace na62 {
namespace test {

FileReader::FileReader() {
	// TODO Auto-generated constructor stub

}

FileReader::~FileReader() {
	// TODO Auto-generated destructor stub
}

/**
 * Generates a HeaderData object by reading the given file
 */
HeaderData FileReader::readHeaderFile(boost::filesystem::path filePath) {
	std::ifstream file;
	std::string fileLine;
	std::vector<std::string> headerRawData(3);
	std::vector<std::string> eventLine(2);
	HeaderData headerData;

	file.open(filePath.string());

	/*
	 * First line: $dataFileName
	 */
	getline(file, fileLine);
	headerData.binaryFile = fileLine;

	/*
	 * Second line: $sourceID:$numberOfReadOutBoards:$numberOfEvents
	 */
	getline(file, fileLine);
	boost::algorithm::split(headerRawData, fileLine, boost::is_any_of(":"));
	if (headerRawData.size() != 3) {
		LOG_ERROR<< "Error after reading header file line: " << fileLine
		<< ENDL;
		LOG_ERROR
		<< "The second line of a header file must have 3 colon-separated strings: $sourceID:$numberOfReadOutBoards:$numberOfEvents"
		<< ENDL;
		exit(1);
	}
	headerData.sourceID = Utils::ToUInt(headerRawData[0]);
	headerData.numberOfReadOutBoards = Utils::ToUInt(headerRawData[1]);
	headerData.numberOfEvents = Utils::ToUInt(headerRawData[2]);

	/*
	 * Following lines: $eventLengthN:$listOfROBDataLengthsN
	 */
	int eventsToRead = headerData.numberOfEvents;

	if (MyOptions::GetInt(OPTION_MAX_EVENT_NUM) != 0) {
		eventsToRead = std::min(MyOptions::GetInt(OPTION_MAX_EVENT_NUM),
				headerData.numberOfEvents);
	}

	for (int i = 0; i != eventsToRead; i++) {
		SubEventHdr subEvent;
		getline(file, fileLine);
		boost::algorithm::split(eventLine, fileLine, boost::is_any_of(":"));
		/*
		 * First column: total event length (sum of all sources), timestamp
		 */
		std::vector<std::string> eventInfo(2);
		boost::algorithm::split(eventInfo, eventLine[0], boost::is_any_of(","));
		subEvent.eventLength = Utils::ToUInt(eventInfo[0]);
		subEvent.timestamp = Utils::ToUInt(eventInfo[1]);

		/*
		 * Second column: list of number of 32bit-words for each ROB event fragment
		 */
		std::vector<std::string> fragmentLengths(
				headerData.numberOfReadOutBoards);
		boost::algorithm::split(fragmentLengths, eventLine[1],
				boost::is_any_of(","));

		for (std::string& length : fragmentLengths) {
			subEvent.ROBDataLengths.push_back(Utils::ToUInt(length));
		}
		headerData.subevents.push_back(std::move(subEvent));
	}

	file.close();

	return headerData;
}

/**
 * Reads the binary files and copies all events into MEPs
 */
void FileReader::readDataFromFile(HeaderData header,
		std::function<void(l0::MEP_HDR*)> finishedMEPCallback) {

	/*
	 * Check if the binary file is an absolute path. If not it's relative to the RAW_INTPUT_DIR path
	 */
	std::string binaryFile = header.binaryFile;

	std::ifstream file(binaryFile, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		LOG_ERROR<< "Unable to open file " << header.binaryFile << ENDL;
		exit(1);
	}

	/*
	 * buffers[i] contains the MEP of ROB i
	 */
	char* buffers[header.numberOfReadOutBoards];
	for (int i = 0; i < header.numberOfReadOutBoards; i++) {
		buffers[i] = nullptr;
	}

	int eventNumber = 0;
	for (SubEventHdr& subEvent : header.subevents) {

		for (int sourceSubID = 0; sourceSubID != header.numberOfReadOutBoards;
				sourceSubID++) {
			char* buffer = buffers[sourceSubID];

			const int fragmentSize = subEvent.ROBDataLengths[sourceSubID];

			l0::MEP_HDR* mep = (l0::MEP_HDR*) buffer;

			/*
			 * Check if the MEP is not yet initialized or already full
			 */
			if (buffer == nullptr
					|| (mep->mepLength + sizeof(l0::MEPFragment_HDR)
							+ fragmentSize * 4 > MaxMEPSize)) {
				// Send the finished MEP to the callback
				if (buffer != nullptr) {
					finishedMEPCallback(mep);
				}

				// create a new MEP
				buffer = new char[MaxMEPSize];
				mep = (l0::MEP_HDR*) buffer;
				mep->sourceID = header.sourceID;
				mep->sourceSubID = sourceSubID;
				mep->eventCount = 0;
				mep->mepLength = sizeof(l0::MEP_HDR);
				mep->firstEventNum = eventNumber;
				buffers[sourceSubID] = buffer;
			}

			/*
			 * Write the fragment header
			 */
			l0::MEPFragment_HDR* fragmentHdr = (l0::MEPFragment_HDR*) (buffer
					+ mep->mepLength);

			fragmentHdr->eventLength_ = sizeof(l0::MEPFragment_HDR)
					+ fragmentSize * 4;

			fragmentHdr->eventNumberLSB_ = eventNumber;
			fragmentHdr->lastEventOfBurst_ = 0;
			fragmentHdr->timestamp_ = subEvent.timestamp;

			/*
			 * Write the raw data to the MEP
			 */
			file.read(buffer + (mep->mepLength + sizeof(l0::MEPFragment_HDR)),
					fragmentSize * 4);

			/*
			 * Update the MEP header
			 */
			mep->eventCount++;
			mep->mepLength += fragmentHdr->eventLength_;
		}
		eventNumber++;
	}

	LOG_INFO<< "Found "<< eventNumber << " events in the binary file " << binaryFile << ENDL;

	/*
	 * Send all remaining MEPs to the callback
	 */
	for (int sourceSubID = 0; sourceSubID != header.numberOfReadOutBoards;
			sourceSubID++) {
		char* buffer = buffers[sourceSubID];
		l0::MEP_HDR* mep = (l0::MEP_HDR*) buffer;
		finishedMEPCallback(mep);
	}

	file.close();
}

/**
 * Returns the header data of all activated sourceIDs
 */
std::vector<HeaderData> FileReader::getActiveHeaderData(
		std::vector<int> sourceIDs, std::vector<std::string> headerFiles) {
	std::vector<HeaderData> headers;

	for (auto& path : headerFiles) {
		if (boost::filesystem::is_regular(path)) {
			HeaderData headerData = readHeaderFile(path);
			for (int sourceID : sourceIDs) {
				if (sourceID == headerData.sourceID) {
					headers.push_back(std::move(headerData));
					break;
				}
			}
		}
	}

	return headers;
}
} /* namespace test */
} /* namespace na62 */
