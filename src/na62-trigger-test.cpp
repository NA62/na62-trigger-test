//============================================================================
// Name        : NA62 online trigger algorithm test
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================

#include <eventBuilding/Event.h>
#include <eventBuilding/SourceIDManager.h>
#include <l1/L1TriggerProcessor.h>
#include <vector>

#include <l0/MEP.h>

#include "options/MyOptions.h"

using namespace na62;
int main(int argc, char* argv[]) {
	/*
	 * Static Class initializations
	 */
	MyOptions::Load(argc, argv);
	l0::MEP* mep = new l0::MEP(nullptr, 0, nullptr);

	Event* e = new Event(0);
	L1TriggerProcessor t;
	SourceIDManager::Initialize(SOURCE_ID_LAV, { }, { });
	return 0;
}
