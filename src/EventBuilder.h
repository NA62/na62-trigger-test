/*
 * EventBuilder.h
 *
 *  Created on: Jul 22, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#ifndef EVENTBUILDER_H_
#define EVENTBUILDER_H_

#include <eventBuilding/Event.h>
#include <vector>
#include <structs/DataContainer.h>

namespace na62 {
class BurstFileWriter;
} /* namespace na62 */

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
	DataContainer originaldata_;

	/*
	 * Processes a single MEPFragment
	 */
	void buildL1(l0::MEPFragment* fragment);
	void processL1(Event* event);
	void processL2(Event* event);

public:
	EventBuilder();
	virtual ~EventBuilder();

	/*
	 * Processes all fragments of the MEP
	 */
	void buildMEP(l0::MEP_HDR* mepHDR);

	std::vector<Event*> getFinishedEvents(){
		return eventPool_;
	}
};
} /* namespace test */
} /* namespace na62 */

#endif /* EVENTBUILDER_H_ */
