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

#define OPTION_RAW_INPUT_DIR (char*)"rawInputPath"
#define OPTION_ACTIVE_SOURCE_IDS (char*)"activeSourceIDs"

#define OPTION_MAX_EVENT_NUM (char*)"maxEventNum"

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

		(OPTION_RAW_INPUT_DIR, po::value<std::string>()->required(),
				"Path to the directory containing all the raw binary files to be read")

		(OPTION_ACTIVE_SOURCE_IDS, po::value<std::string>()->required(),
				"List of Source IDs to be used")

		(OPTION_MAX_EVENT_NUM, po::value<int>()->default_value(0),
				"Maximum number of events to be processed. If set to 0 all events found will be processed");

		Options::Initialize(argc, argv, desc);
	}
};
} /* namespace test */
} /* namespace na62 */

#endif /* MYOPTIONS_H_ */
