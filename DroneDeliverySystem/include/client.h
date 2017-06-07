#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_CLIENT_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_CLIENT_H

#include <mqueue.h>
#include <stdbool.h>
#include "typedefs.h"

struct client {
	unsigned int id;
	unsigned int distance; //position with (x,y)?
	bool targetInstalled;
	mqd_t msgQueueID;
};

Client* client_constructor(unsigned int distance);

void client_free(Client* client);

void* client_launch(/* Client* */ void* client);

void client_sendMessage(Client* client, ClientMessage* message);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_CLIENT_H
