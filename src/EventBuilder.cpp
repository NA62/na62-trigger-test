/*
 * EventBuilder.cpp
 *
 *  Created on: Jul 22, 2014
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventBuilder.h"

namespace na62 {
namespace test {
EventBuilder::EventBuilder() {
	eventPool_.resize(1000);
}

EventBuilder::~EventBuilder() {
}
} /* namespace test */
} /* namespace na62 */
