#include "drone.h"

#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include <unistd.h> //FIXME: sleep for test

#include "drone_message.h"
#include "mothership.h"
#include "mothership_message.h"

static unsigned int numberOfDrone = 0;
static struct mq_attr attr;

static void process_message(Drone* drone, DroneMessage* message);

Drone* drone_constructor(unsigned int maxLoad, unsigned int autonomy, unsigned int rechargingTime, Mothership* motherShip) {
	Drone* drone = (Drone*) malloc(sizeof(Drone));
	drone->id = numberOfDrone++;
	drone->maxLoad = maxLoad;
	drone->autonomy = autonomy;
	drone->rechargingTime = rechargingTime;

	char buffer[10];
	sprintf(buffer, "/drone%03d", drone->id);

	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(DroneMessage);
	if((drone->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)) == -1) {
		char errorBuffer[30];
		sprintf(errorBuffer, "Could not create drone %03d", drone->id);
		perror(errorBuffer);

		free(drone);
		return NULL;
	}

	if(mq_unlink(buffer) == -1) {
		mq_close(drone->msgQueueID);
		free(drone);

		return NULL;
	}

	drone->motherShip = motherShip;
	drone->client = NULL;

	return drone;
}

void drone_free(Drone* drone) {
	if(mq_close(drone->msgQueueID) == -1) {
		char errorBuffer[30];
		sprintf(errorBuffer, "Could not destroy drone %03d", drone->id);
		perror(errorBuffer);
	}

	free(drone);
}

void* drone_launch(/* Drone* */ void* drone) {
	//FIXME: test start
	Drone* drone1 = (Drone*) drone;
	sleep(drone1->id);
	while(true) {
		MothershipMessage mothershipMessage;
		mothershipMessage.sender_id = drone1->id;
		mothershipMessage.type = DRONE_PACKAGE_DELIVERED_SUCCESS;
		mothership_sendMessage(drone1->motherShip, &mothershipMessage);
		printf("[Drone %d] Sent message to mothership\n", drone1->id);

		DroneMessage droneMessage;
		check((int) mq_receive(drone1->msgQueueID, (char*) &droneMessage, sizeof(DroneMessage), 0),
		      "mq_receive failed");
		printf("[Drone %d] received message from mothership\n", drone1->id);
		sleep(4);
	}
	//FIXME: test end
	return NULL; // TODO: pthread_exit
}

void drone_sendMessage(Drone* drone, DroneMessage* message) {
	check(mq_send(drone->msgQueueID, (const char*) message, sizeof(DroneMessage), 0), "mq_send failed");
}

void process_message(Drone* drone, DroneMessage* message) {
	switch (message->type) {
		case MOTHERSHIP_END_OF_DELIVERY:
			LOG_INFO("Drone %03d poweroff", drone->id);
			pthread_exit(0);
			break;
		case MOTHERSHIP_GO_RECHARGE:
			{
				sleep(drone->rechargingTime); // TODO: Maybe the drone should waits messages.
				MothershipMessage mothershipMessage;
				mothershipMessage.sender_id = drone->id;
				mothershipMessage.type = DRONE_DONE_CHARGING;
			}
			break;
		case MOTHERSHIP_GO_DELIVER_PACKAGE:
			// TODO
			break;
		default:
			// TODO
			break;
	}
}
