/*
 * EventBuilder.cpp
 *
 *  Created on: Jul 22, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventBuilder.h"

#include <eventBuilding/SourceIDManager.h>
#include <l0/MEP.h>
#include <l0/MEPFragment.h>
#include <l0/Subevent.h>
#include <l1/L1TriggerProcessor.h>
#include <l2/L2TriggerProcessor.h>
#include <sys/types.h>
#include <cstdint>
#include <structs/Event.h>

namespace na62 {
namespace test {

std::atomic<uint64_t> EventBuilder::L1AcceptedEvents_(0);
std::atomic<uint64_t> EventBuilder::L1AllAlgoDisabledEvents_(0);
std::atomic<uint64_t> EventBuilder::L1BypassedEvents_(0);
std::atomic<uint64_t> EventBuilder::L1FlaggedAlgoProcessedEvents_(0);
std::atomic<uint64_t> EventBuilder::L1AutoPassFlagEvents_(0);

EventBuilder::EventBuilder() {
}

EventBuilder::~EventBuilder() {
	for (auto& event : eventPool_) {
		if (event) {
			delete event;
		}
	}
}

void EventBuilder::buildL1(l0::MEPFragment* fragment) {
	Event* event;
	if (fragment->getEventNumber() >= eventPool_.size()) { // Memory overflow
		eventPool_.resize(fragment->getEventNumber() + 1);
		event = new Event(fragment->getEventNumber());
		eventPool_[fragment->getEventNumber()] = event;
	} else {
		event = eventPool_[fragment->getEventNumber()];
		if (event == nullptr) { // An event with a higher eventPoolIndex has been received before this one
			event = new Event(fragment->getEventNumber());
			eventPool_[fragment->getEventNumber()] = event;
		}
	}

	if (event->addL0Fragment(fragment, 0)) {
		processL1(event);
	}
}

void EventBuilder::buildMEP(l0::MEP_HDR* mepHDR) {
//	l0::MEP* mep = new l0::MEP((char*) mepHDR, mepHDR->mepLength, (char*) mepHDR);
	l0::MEP* mep = new l0::MEP((char*) mepHDR, mepHDR->mepLength,
			originaldata_);

	for (uint i = 0; i != mep->getNumberOfFragments(); i++) {
		l0::MEPFragment* fragment = mep->getFragment(i);
		buildL1(fragment);
	}
}

void EventBuilder::processL1(Event* event) {
	/*
	 * Read the L0 trigger type word and the fine time from the L0TP data
	 */
	const uint_fast8_t l0TriggerTypeWord =
			event->readTriggerTypeWordAndFineTime();

	/*
	 * Store the global event timestamp taken from the reverence detector
	 * Store fine time taken from the reference detector (temporary solution)
	 */
	l0::MEPFragment* tsFragment = event->getL0SubeventBySourceIDNum(
			SourceIDManager::TS_SOURCEID_NUM)->getFragment(0);
	event->setTimestamp(tsFragment->getTimestamp());
	///////////////// Temporary Modification to store L0TP reference detector finetime ///////////////////////
	event->setFinetime(tsFragment->getDataWithMepHeader()->reserved_);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////


	/*
	 * Process Level 1 trigger with all options: reduction, downscaling, bypassing
	 */
//	printf("EventBuilder.cpp: l0TriggerTypeWord %x\n", (uint)l0TriggerTypeWord);
	uint_fast8_t l1TriggerTypeWord = L1TriggerProcessor::compute(event);
//	printf("EventBuilder.cpp: l1TriggerTypeWord %x\n", (uint)l1TriggerTypeWord);
	/*
	 * Given the l1TriggerTypeWord you can now count separately the bitset!
	 * bit 0 = for OR of all algo verdicts
	 * bit 4 = isAllAlgoDisable
	 * bit 5 = isBypassed event
	 * bit 6 = is at least one algo being processed in flagging
	 * bit 7 = autoPass/Flag event
	 */

	if(l1TriggerTypeWord & 0x1) L1AcceptedEvents_.fetch_add(1, std::memory_order_relaxed);
	if(l1TriggerTypeWord & TRIGGER_L1_ALLDISABLED) L1AllAlgoDisabledEvents_.fetch_add(1, std::memory_order_relaxed);
	if(l1TriggerTypeWord & TRIGGER_L1_BYPASS) L1BypassedEvents_.fetch_add(1, std::memory_order_relaxed);
	if(l1TriggerTypeWord & TRIGGER_L1_FLAGALGO) L1FlaggedAlgoProcessedEvents_.fetch_add(1, std::memory_order_relaxed);
	if(l1TriggerTypeWord & TRIGGER_L1_AUTOPASS) L1AutoPassFlagEvents_.fetch_add(1, std::memory_order_relaxed);

	uint_fast16_t L0L1Trigger(l0TriggerTypeWord | l1TriggerTypeWord << 8);
//	printf("EventBuilder.cpp: L0L1Trigger %x\n", (uint)L0L1Trigger);

	event->setL1Processed(L0L1Trigger);

	if (l1TriggerTypeWord != 0) {
//		LOG_INFO<< "Process L2!!!" << ENDL;
		processL2(event);
	} else {
//		LOG_INFO<< "Free Event!!!" << ENDL;
		event->destroy();
	}
}

void EventBuilder::processL2(Event* event) {
	uint_fast8_t l2TriggerTypeWord = L2TriggerProcessor::compute(event);
	event->setL2Processed(l2TriggerTypeWord);

	if (!l2TriggerTypeWord) {
//		LOG_INFO<< "Free Event!!!" << ENDL;
		event->destroy();
	}
}

} /* namespace test */
} /* namespace na62 */
