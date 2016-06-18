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
#include <atomic>

#define TRIGGER_L1_ALLDISABLED 0x10
#define TRIGGER_L2_ALLDISABLED 0x10
#define TRIGGER_L1_FLAGALGO 0x40
#define TRIGGER_L2_FLAGALGO 0x40
#define TRIGGER_L1_AUTOPASS 0x80
#define TRIGGER_L2_AUTOPASS 0x80

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
	static std::atomic<uint64_t> L1AcceptedEvents_;
	static std::atomic<uint64_t> L1AllAlgoDisabledEvents_;
	static std::atomic<uint64_t> L1BypassedEvents_;
	static std::atomic<uint64_t> L1FlaggedAlgoProcessedEvents_;
	static std::atomic<uint64_t> L1AutoPassFlagEvents_;

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

	std::vector<Event*> getFinishedEvents() {
		return eventPool_;
	}

	static inline uint64_t GetL1AcceptedStats() {
		return L1AcceptedEvents_;
	}
	static inline uint64_t GetL1AllAlgoDisabledStats() {
		return L1AllAlgoDisabledEvents_;
	}
	static inline uint64_t GetL1BypassedStats() {
		return L1BypassedEvents_;
	}
	static inline uint64_t GetL1FlaggedAlgoProcessedStats() {
		return L1FlaggedAlgoProcessedEvents_;
	}
	static inline uint64_t GetL1AutoPassFlagStats() {
		return L1AutoPassFlagEvents_;
	}
};
} /* namespace test */
} /* namespace na62 */

#endif /* EVENTBUILDER_H_ */
