/*
 * EventBuilder.cpp
 *
 *  Created on: Jul 22, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventBuilder.h"

#include <eventBuilding/SourceIDManager.h>
#include <eventBuilding/Event.h>
#include <l0/MEP.h>
#include <l0/MEPFragment.h>
#include <l0/Subevent.h>
#include <l1/L1TriggerProcessor.h>
#include <l2/L2TriggerProcessor.h>
#include <monitoring/HltStatistics.h>
#include <sys/types.h>
#include <cstdint>
#include <structs/Event.h>
#include <structs/L0TPHeader.h>

namespace na62 {
namespace test {

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
	l0::MEP* mep = new l0::MEP((char*) mepHDR, mepHDR->mepLength, originaldata_);

	for (uint i = 0; i != mep->getNumberOfFragments(); i++) {
		l0::MEPFragment* fragment = mep->getFragment(i);
		buildL1(fragment);
	}
}

void EventBuilder::processL1(Event* event) {
	/*
	 * Read the L0 trigger type word and the fine time from the L0TP data
	 */
	const uint_fast8_t l0TriggerTypeWord = event->readTriggerTypeWordAndFineTime();

	if (SourceIDManager::L0TP_ACTIVE) {
		l0::MEPFragment* L0TPEvent = (event->getL0TPSubevent())->getFragment(0);
		L0TpHeader* L0TPData = (L0TpHeader*) L0TPEvent->getPayload();
//		LOG_INFO("L0TPData Finetime " << (uint )L0TPData->refFineTime);
//		LOG_INFO("L0TPData DataType " << (uint )L0TPData->dataType);
//		for (uint i = 0; i < 7; i++) {
//			LOG_INFO("L0TPData Primitives " << (uint )L0TPData->primitives[i]);
//		}
//		LOG_INFO("L0TPData previous Timestamp " << (uint )L0TPData->previousTimeStamp);
//		LOG_INFO("L0TPData L0TriggerType " << (uint )L0TPData->l0TriggerType);
//		LOG_INFO("L0TPData previous L0TriggerType " << (uint )L0TPData->previousl0TriggerType);
//		LOG_INFO("L0TPData Trigger Flags " << (uint )L0TPData->l0TriggerFlags);

		event->setFinetime(L0TPData->refFineTime);
		event->setTriggerDataType(L0TPData->dataType);
		event->setl0TriggerTypeWord(L0TPData->l0TriggerType);
		event->setTriggerFlags(L0TPData->l0TriggerFlags);
	}
	/*
	 * Store the global event timestamp taken from the reverence detector
	 * Store fine time taken from the reference detector (temporary solution)
	 */
	l0::MEPFragment* tsFragment = event->getL0SubeventBySourceIDNum(SourceIDManager::TS_SOURCEID_NUM)->getFragment(0);
	event->setTimestamp(tsFragment->getTimestamp());
	///////////////// Temporary Modification to store L0TP reference detector finetime ///////////////////////
//	event->setFinetime(tsFragment->getDataWithMepHeader()->reserved_);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Process Level 1 trigger with all options: reduction, downscaling, bypassing
	 */
	uint_fast8_t l1TriggerTypeWord = L1TriggerProcessor::compute(event, strawAlgo_);
	/*
	 * Given the l1TriggerTypeWord you can now count separately the bitset!
	 * bit 0 = for OR of all algo verdicts
	 * bit 3 = isWhileTimeout
	 * bit 4 = isAllAlgoDisable
	 * bit 5 = isBypassed event
	 * bit 6 = is at least one algo being processed in flagging
	 * bit 7 = autoPass/Flag event
	 */

	// Read L1 info to storage data
	L1InfoToStorage L1Info = L1TriggerProcessor::getL1Info();
//	cout << "KTAG Sectors " << (uint) L1Info.getL1KTAGNSectorsL0TP() << endl;
//	cout << "L0 Trigger Flags " << (uint)event->getTriggerFlags() << endl;

	// STATISTICS
	HltStatistics::updateL1Statistics(event, l1TriggerTypeWord);

	uint_fast16_t L0L1Trigger(l0TriggerTypeWord | l1TriggerTypeWord << 8);
//	printf("EventBuilder.cpp: L0L1Trigger %x\n", (uint)L0L1Trigger);

	event->setL1Processed(L0L1Trigger);

	if (l1TriggerTypeWord != 0) {
		HltStatistics::sumCounter("L1RequestToCreams", 1); // new way to handle counters
		processL2(event);
	} else {
//		LOG_INFO<< "Free Event!!!" << ENDL;
		event->destroy();
	}
}

void EventBuilder::processL2(Event* event) {
	uint_fast8_t l2TriggerTypeWord = L2TriggerProcessor::compute(event);

	/*
	 * Given the l2TriggerTypeWord you can now count separately the bitset!
	 * bit 0 = for OR of all algo verdicts
	 * bit 4 = isAllAlgoDisable
	 * bit 5 = isBypassed event
	 * bit 6 = is at least one algo being processed in flagging
	 * bit 7 = autoPass/Flag event
	 */

	HltStatistics::updateL2Statistics(event, l2TriggerTypeWord);

	event->setL2Processed(l2TriggerTypeWord);

	if (!l2TriggerTypeWord) {
//		LOG_INFO<< "Free Event!!!" << ENDL;
		event->destroy();
	}
}

} /* namespace test */
} /* namespace na62 */
