/*
 * EventBuilder.h
 *
 *  Created on: Jul 22, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_

#include <eventBuilding/Event.h>
#include <l0/MEP.h>
#include <l0/Subevent.h>
#include <l0/MEPFragment.h>
#include <l1/L1TriggerProcessor.h>
#include <vector>

namespace na62 {
namespace l0 {
struct MEP_HDR;
} /* namespace l0 */
} /* namespace na62 */

namespace na62 {
namespace test {

class EventBuilder {
private:
	std::vector<Event*> eventPool_;
public:
	EventBuilder();
	virtual ~EventBuilder();

	void buildEvent(l0::MEPFragment* fragment) {
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
			/*
			 * Set the global event timestamp according to the value of the timestamp subdetector
			 */
			l0::MEPFragment* tsFragment = event->getL0SubeventBySourceID(
					SourceIDManager::TS_SOURCEID_NUM)->getFragment(0);
			event->setTimestamp(tsFragment->getTimestamp());

			/*
			 * Run L1 trigger algorithm
			 */
			L1TriggerProcessor::compute(event);
		}
	}

	void buildMEP(l0::MEP_HDR* mepHDR) {
		l0::MEP* mep = new l0::MEP((char*) mepHDR, mepHDR->mepLength,
				(char*) mepHDR);

		for (int i = mep->getNumberOfEvents() - 1; i != -1; i--) {
			l0::MEPFragment* fragment = mep->getFragment(i);
			buildEvent(fragment);
		}
	}
};
} /* namespace test */
} /* namespace na62 */

#endif /* EVENTBUILDER_H_ */
