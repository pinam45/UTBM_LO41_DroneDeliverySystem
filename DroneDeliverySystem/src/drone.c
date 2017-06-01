#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "drone.h"
#include "drone_message.hpp"

static unsigned int numberOfDrone = 0;
static struct mq_attr attr = { 0, 10, 2048, 0};

Drone* drone_constructor(unsigned int maxLoad, unsigned int autonomy, unsigned int rechargingTime, Mothership* motherShip) {
	Drone* drone = (Drone*)malloc(sizeof(Drone));
	drone->id = numberOfDrone++;
	drone->maxLoad = maxLoad;
	drone->autonomy = autonomy;
	drone->rechargingTime = rechargingTime;

	char buffer[10];
	sprintf(buffer, "/drone%03d", drone->id);

	if ((drone->msgQueueID = mq_open(buffer, O_WRONLY | O_CREAT, 0660, &attr)) == -1) {
		char errorBuffer[30];
		sprintf(errorBuffer, "Could not create drone %03d", drone->id);
		perror(errorBuffer);

		free(drone);
		return NULL;
	}

	if (mq_unlink(buffer) == -1) {
		mq_close(drone->msgQueueID);
		free(drone);

		return NULL;
	}

	drone->motherShip = motherShip;
	drone->client = NULL;

	return drone;
}

void drone_free(Drone* drone) {
	if (mq_close(drone->msgQueueID) == -1) {
		char errorBuffer[30];
		sprintf(errorBuffer, "Could not destroy drone %03d", drone->id);
		perror(errorBuffer);
	}

	free(drone);
}

void drone_sendMessage(Drone* drone, DroneMessage* message) {
	check(mq_send(drone->msgQueueID, (const char*) message, sizeof(DroneMessage), 0), "mq_send failed");
}
