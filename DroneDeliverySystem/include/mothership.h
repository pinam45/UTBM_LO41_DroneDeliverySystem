#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_H

#include <mqueue.h>
#include <pthread.h>

#include "LinkedList.h"
#include "typedefs.h"

struct mothership {
	LinkedList* droneList;
	LinkedList* availableDrones;
	LinkedList* clientList;
	LinkedList* packageList;
	mqd_t msgQueueID;
	size_t numberOfPackages;
	unsigned int deadDronesNbr;
};

Mothership* mothership_constructor(LinkedList* droneList, LinkedList* clientList, LinkedList* packageList);

void mothership_free(Mothership* mothership);

void mothership_launch(Mothership* mothership);

void mothership_sendMessage(Mothership* mothership, MothershipMessage* message);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_H
