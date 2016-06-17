/*
 * FileReader.h
 *
 *  Created on: Jul 22, 2014
 *  	Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef FILEREADER_H_
#define FILEREADER_H_

#include <boost/filesystem/path.hpp>
#include <l0/MEP.h>
#include <functional>
#include <string>
#include <vector>

#define MaxMEPSize MTU-14/*ethernet*/-20/*ip*/-8/*udp*/

namespace na62 {
namespace test {
/**
 * Data to store pointers to the raw data of all ROBs of one event
 */
typedef struct {
	int eventLength; // Number of long words (32 bit) used to store the current event
	uint timestamp;
	///////////////// Temporary Modification to store L0TP reference detector finetime ///////////////////////
	uint finetime;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::vector<int> ROBDataLengths;
} SubEventHdr;

typedef struct {
	std::string binaryFile;
	int sourceID;
	int numberOfReadOutBoards;
	int numberOfEvents;
	std::vector<SubEventHdr> subevents;
} HeaderData;

class FileReader {
public:
	FileReader();
	virtual ~FileReader();

	/**
	 * Generates a HeaderData object by reading the given file
	 */
	static HeaderData readHeaderFile(std::string filePath);

	/**
	 * Reads the binary files and copies all events into MEPs. The callback is called for every MEP created
	 */
	static void readDataFromFile(HeaderData header,
			std::function<void(l0::MEP_HDR*)> finishedMEPCallback);

	/**
	 * Returns the header data of all activated sourceIDs
	 */
	static std::vector<HeaderData> getActiveHeaderData(
			std::vector<int> sourceIDs, std::vector<std::string> headerFiles);

};
} /* namespace test */
} /* namespace na62 */

#endif /* FILEREADER_H_ */
