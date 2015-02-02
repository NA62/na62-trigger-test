/*
 * MyOptions.h
 *
 *  Created on: Apr 11, 2014
 \*      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef MYOPTIONS_H_
#define MYOPTIONS_H_

#include <options/Options.h>
#include <string>

/*
 * Available options
 */

#define OPTION_ACTIVE_SOURCE_IDS (char*)"activeSourceIDs"

#define OPTION_MAX_EVENT_NUM (char*)"maxEventNum"

#define OPTION_TS_SOURCEID (char*)"timestampSourceID"

namespace na62 {
namespace test {
class MyOptions: public Options {
public:
	MyOptions();
	virtual ~MyOptions();

	static void Load(int argc, char* argv[]) {
		desc.add_options()

		(OPTION_CONFIG_FILE,
				po::value<std::string>()->default_value(
						"na62-trigger-test.conf"),
				"Configuration file for the options shown here")

		(OPTION_ACTIVE_SOURCE_IDS, po::value<std::string>()->required(),
				"List of Source IDs to be used")

		(OPTION_MAX_EVENT_NUM, po::value<int>()->default_value(0),
				"Maximum number of events to be processed. If set to 0 all events found will be processed")

		(OPTION_TS_SOURCEID, po::value<std::string>()->required(),
				"Source ID of the detector whose timestamp should be written into the final event and sent to the LKr for L1-triggers.")

				;
		Options::Initialize(argc, argv, desc);
	}
};
} /* namespace test */
} /* namespace na62 */

#endif /* MYOPTIONS_H_ */
