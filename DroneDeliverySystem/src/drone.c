#include "drone.h"

#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include <unistd.h> //FIXME: sleep for test
#include <client.h>

#include "drone_message.h"
#include "mothership.h"
#include "mothership_message.h"
static struct mq_attr attr;

static void process_message(Drone* drone, DroneMessage* message);
static unsigned int compute_sleep_time(Drone* drone);

Drone* drone_constructor(unsigned int id, unsigned int maxLoad, unsigned int autonomy, unsigned int rechargingTime, Mothership* motherShip) {
	Drone* drone = (Drone*) malloc(sizeof(Drone));
	drone->id = id;
	drone->maxLoad = maxLoad;
	drone->maxAutonomy = drone->autonomy = autonomy;
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
	drone->package = NULL;
	drone->deliverySucess = false;
	drone->state = S_IN_MOTHERSHIP;

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

void* drone_launch(Drone* drone) {
	while (true) {
		DroneMessage droneMessage;
		check((int) mq_receive(drone->msgQueueID, (char*) &droneMessage, sizeof(DroneMessage), 0), "mq_receive failed");

		process_message(drone, &droneMessage);
	}

	pthread_exit(0);
}

void drone_sendMessage(Drone* drone, DroneMessage* message) {
	check(mq_send(drone->msgQueueID, (const char*) message, sizeof(DroneMessage), 0), "mq_send failed");
}

unsigned int computePowerConsumption(Drone* drone, Package* package, double mothershipToClientDistance) {
	return package->weight + (unsigned int) mothershipToClientDistance;
}

void process_message(Drone* drone, DroneMessage* message) {
	switch (message->type) {
		case MOTHERSHIP_END_OF_DELIVERY:
			LOG_INFO("[Drone %03d] poweroff", drone->id);
			pthread_exit(0);
			break;
		case MOTHERSHIP_GO_RECHARGE:
			{
				LOG_INFO("[Drone %03d] Recharging battery", drone->id);
				sleep(drone->rechargingTime);
				drone->autonomy = drone->maxAutonomy;
				MothershipMessage mothershipMessage;
				mothershipMessage.sender_id = drone->id;
				mothershipMessage.type = DRONE_DONE_CHARGING;

				LOG_INFO("[Drone %03d] Battery charged", drone->id);

				mothership_sendMessage(drone->motherShip, &mothershipMessage);
			}
			break;
		case MOTHERSHIP_GO_DELIVER_PACKAGE:
			LOG_INFO("[Drone %03d] Package", drone->id);
			unsigned int consumption = computePowerConsumption(drone, drone->package, 5);

			sleep(compute_sleep_time(drone));

			MothershipMessage msg;
			msg.sender_id = drone->id;
			if (drone->id != 2) {
				msg.type = DRONE_PACKAGE_DELIVERED_SUCCESS;
				// TODO: The client should do this
				package_free(drone->package);
			} else {
				msg.type = DRONE_PACKAGE_DELIVERED_FAIL;
			}

			check(mq_send(drone->motherShip->msgQueueID, (char*)&msg, sizeof(MothershipMessage), 0), "failed to send a message to the mothership");

			sleep(compute_sleep_time(drone));
			drone->autonomy -= consumption;

			msg.type= DRONE_BACK_TO_MOTHERSHIP;
			check(mq_send(drone->motherShip->msgQueueID, (char*)&msg, sizeof(MothershipMessage), 0), "failed to send a message to the mothership");
			break;
		default:
			LOG_INFO("[Drone %03d] TODO", drone->id);
			//pthread_exit(NULL);
			break;
	}
}

unsigned int compute_sleep_time(Drone* drone) {
	// FIXME: The client isn't set.
	//unsigned int distance = drone->client->distance / 4;
	unsigned int distance = 4;

	return distance > 0 ? 1 : distance;
}
