/*
 * EventBuilder.h
 *
 *  Created on: Jul 22, 2014
 *      Author: root
 */

#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_

#include <eventBuilding/Event.h>
#include <l0/MEP.h>
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
		L1TriggerProcessor t;

		if (event->addL0Event(fragment, 0)) {
			t.compute(event);
		}
	}

	void buildMEP(l0::MEP_HDR* mepHDR) {

		l0::MEP* mep = new l0::MEP((char*) mepHDR, mepHDR->mepLength,
				(char*) mepHDR);

		for (int i = mep->getNumberOfEvents() - 1; i != -1; i--) {
			l0::MEPFragment* fragment = mep->getEvent(i);
			buildEvent(fragment);
		}
	}
};
} /* namespace test */
} /* namespace na62 */

#endif /* EVENTBUILDER_H_ */
