/*
 * EventBuilder.cpp
 *
 *  Created on: Jul 22, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventBuilder.h"

#include <eventBuilding/SourceIDManager.h>
#include <eventBuilding/EventSerializer.h>
#include <l0/MEPFragment.h>
#include <l0/MEP.h>
#include <l0/Subevent.h>
#include <l1/L1TriggerProcessor.h>
#include <l2/L2TriggerProcessor.h>
#include <cstdint>
#include <structs/Event.h>
#include <string>

#include <utils/DataDumper.h>

#include "options/MyOptions.h"

namespace na62 {
namespace test {

EventBuilder::EventBuilder() :
		outputFile_(nullptr) {
	eventPool_.resize(1000);

	std::string fileName = Options::GetString(OPTION_RAW_FILE_NAME);
	if (!fileName.empty()) {
		std::string outputDir = Options::GetString(OPTION_OUTPUT_DIR);

		DataDumper::generateDirIfNotExists(outputDir);
		std::string filePath = DataDumper::generateFreeFilePath(fileName,
				outputDir);

		outputFile_->open(filePath,
				std::ios::out | std::ios::trunc | std::ios::binary);

		if (!outputFile_->good()) {
			LOG_ERROR<< "Unable to write to file " << filePath;
			exit(1);
		}
	}
}

EventBuilder::~EventBuilder() {
	if (outputFile_ != nullptr) {
		outputFile_->close();
		delete outputFile_;
	}
}

void EventBuilder::buildL1(l0::MEPFragment* fragment) {
	Event* event;
	if (fragment->getEventNumber() >= eventPool_.size()) { // Memory overflow
		eventPool_.resize(fragment->getEventNumber() * 2);
		event = new Event(fragment->getEventNumber());
		eventPool_[fragment->getEventNumber()] = event;
	} else {
		event = eventPool_[fragment->getEventNumber()];
		if (event == nullptr) { // An event with a higher eventPoolIndex has been received before this one
			event = new Event(fragment->getEventNumber());
			eventPool_[fragment->getEventNumber()] = event;
		}
	}

	if (event->addL0Event(fragment, 0)) {
		processL1(event);
	}
}

void EventBuilder::buildMEP(l0::MEP_HDR* mepHDR) {
	l0::MEP* mep = new l0::MEP((char*) mepHDR, mepHDR->mepLength,
			(char*) mepHDR);

	for (int i = 0; i != mep->getNumberOfEvents(); i++) {
		l0::MEPFragment* fragment = mep->getFragment(i);
		buildL1(fragment);
	}
}

void EventBuilder::processL1(Event* event) {
	/*
	 * Read the L0 trigger type word and the fine time from the L0TP data
	 */
	const uint8_t l0TriggerTypeWord = event->readTriggerTypeWordAndFineTime();

	/*
	 * Store the global event timestamp taken from the reverence detector
	 */
	l0::MEPFragment* tsFragment = event->getL0SubeventBySourceIDNum(
			SourceIDManager::TS_SOURCEID_NUM)->getFragment(0);
	event->setTimestamp(tsFragment->getTimestamp());

	/*
	 * Process Level 1 trigger
	 */
	uint8_t l1TriggerTypeWord = L1TriggerProcessor::compute(event);
	uint16_t L0L1Trigger(l0TriggerTypeWord | l1TriggerTypeWord << 8);

	event->setL1Processed(L0L1Trigger);

	if (L0L1Trigger != 0) {
		processL2(event);
	} else {
		delete event;
	}
}

void EventBuilder::processL2(Event* event) {
	uint8_t l2TriggerTypeWord = L2TriggerProcessor::compute(event);
	event->setL2Processed(l2TriggerTypeWord);

	if (l2TriggerTypeWord) {
		if (outputFile_ != nullptr) {
			const EVENT_HDR* data = EventSerializer::SerializeEvent(event);
			outputFile_->write((char*) data, data->length * 4);
		}
	}
	delete event;
}

} /* namespace test */
} /* namespace na62 */
