//============================================================================
// Name        : NA62 online trigger algorithm test
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================

#include <eventBuilding/Event.h>
#include <l1/L1TriggerProcessor.h>
#include "../options/MyOptions.h"

using namespace na62;
int main(int argc, char* argv[]) {
	/*
	 * Static Class initializations
	 */
	Options::Initialize(argc, argv);

	Event* e = new Event(0);
	L1TriggerProcessor t;
	return 0;
}
