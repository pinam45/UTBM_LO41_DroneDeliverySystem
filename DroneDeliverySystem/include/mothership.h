#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_HPP
#define UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_HPP

#include <mqueue.h>

#include "LinkedList.h"
#include "typedefs.h"

struct mothership {
	LinkedList* droneList;
	LinkedList* clientList;
	LinkedList* packageList;
	mqd_t msgQueueID;
};

Mothership* mothership_constructor(LinkedList* droneList, LinkedList* clientList, LinkedList* packageList);

void mothership_free(Mothership* mothership);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_HPP
