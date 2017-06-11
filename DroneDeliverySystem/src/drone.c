#include "drone.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"
#include "client.h"
#include "dashboard.h"
#include "client_message.h"
#include "drone_message.h"
#include "mothership.h"
#include "mothership_message.h"

static bool process_message(Drone* drone, DroneMessage* message);

static unsigned int compute_sleep_time(Drone* drone);

Drone* drone_constructor(unsigned int id, unsigned int maxLoad, unsigned int autonomy,
                         unsigned int rechargingTime, Mothership* motherShip) {
	Drone* drone = (Drone*) malloc(sizeof(Drone));
	drone->id = id;
	drone->maxLoad = maxLoad;
	drone->maxAutonomy = drone->autonomy = autonomy;
	drone->rechargingTime = rechargingTime;
	drone->motherShip = motherShip;
	drone->client = NULL;
	drone->package = NULL;
	drone->deliverySuccess = false;
	drone->state = S_ALIVE;

	check(pthread_mutex_init(&(drone->mutex), NULL),"Drone: pthread_mutex_init failed");

	char buffer[10];
	sprintf(buffer, "/drone%03d", drone->id);
	struct mq_attr attr;
	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(DroneMessage);
	check((drone->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)), "Drone: mq_open failed");
	check(mq_unlink(buffer), "Drone: mq_unlink failed");

	return drone;
}

void drone_free(Drone* drone) {
	check(pthread_mutex_destroy(&(drone->mutex)),"Drone: pthread_mutex_destroy failed");
	check(mq_close(drone->msgQueueID),"Drone: mq_close failed");
	free(drone);
}

void* drone_launch(Drone* drone) {
	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_DRONE;
	dashboardMessage.number = drone->id;
	dashboardMessage.state = D_DRONE_WAITING;
	dashboard_sendMessage(global_dashboard, &dashboardMessage);

	DroneMessage droneMessage;
	check((int) mq_receive(drone->msgQueueID, (char*) &droneMessage, sizeof(DroneMessage), 0), "mq_receive failed");
	while(process_message(drone, &droneMessage)) {
		check((int) mq_receive(drone->msgQueueID, (char*) &droneMessage, sizeof(DroneMessage), 0), "mq_receive failed");
	}
	pthread_exit(0);
}

void drone_sendMessage(Drone* drone, DroneMessage* message) {
	check(mq_send(drone->msgQueueID, (const char*) message, sizeof(DroneMessage), 0), "mq_send failed");
}

unsigned int computePowerConsumption(Drone* drone, Package* package, double mothershipToClientDistance) {
	return ((unsigned int)abs((int)package->weight - (int)drone->maxLoad) / 4) + (unsigned int) mothershipToClientDistance;
}

bool process_message(Drone* drone, DroneMessage* message) {
	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_DRONE;
	dashboardMessage.number = drone->id;

	switch(message->type) {
		case MOTHERSHIP_END_OF_DELIVERY: {
			dashboardMessage.state = D_DRONE_FINISHED;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			pthread_mutex_lock(&(drone->mutex));
			drone->state = S_DEAD;
			pthread_mutex_unlock(&(drone->mutex));
			LOG_INFO("[Drone %03d] poweroff", drone->id);
			return false;
		}
		case MOTHERSHIP_GO_RECHARGE: {
			dashboardMessage.state = D_DRONE_CHARGING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			LOG_INFO("[Drone %03d] Recharging battery", drone->id);
			sleep(drone->rechargingTime);
			pthread_mutex_lock(&(drone->mutex));
			drone->autonomy = drone->maxAutonomy;
			pthread_mutex_unlock(&(drone->mutex));

			LOG_INFO("[Drone %03d] Battery charged", drone->id);

			dashboardMessage.state = D_DRONE_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			MothershipMessage mothershipMessage;
			mothershipMessage.sender_id = drone->id;
			mothershipMessage.type = DRONE_DONE_CHARGING;
			mothership_sendMessage(drone->motherShip, &mothershipMessage);
			break;
		}
		case MOTHERSHIP_GO_DELIVER_PACKAGE: {
			dashboardMessage.state = D_DRONE_FLYING_MTC;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			LOG_INFO("[Drone %03d] Package", drone->id);
			pthread_mutex_lock(&(drone->mutex));
			unsigned int consumption = computePowerConsumption(drone, drone->package, 5);
			pthread_mutex_unlock(&(drone->mutex));

			sleep(compute_sleep_time(drone));
			ClientMessage clientMessage;
			clientMessage.type = DRONE_PUT_TARGET;
			pthread_mutex_lock(&(drone->mutex));
			client_sendMessage(drone->client, &clientMessage);
			pthread_mutex_unlock(&(drone->mutex));
			sleep(1);

			MothershipMessage mothershipMessage;
			mothershipMessage.sender_id = drone->id;
			pthread_mutex_lock(&(drone->mutex));
			pthread_mutex_lock(&(drone->client->targetMutex));
			if(drone->client->targetInstalled) {
				dashboardMessage.state = D_DRONE_FLYING_CTM_DELIVERY_SUCCESS;
				mothershipMessage.type = DRONE_PACKAGE_DELIVERED_SUCCESS;
				clientMessage.type = DRONE_DELIVERY_SUCCESS;
				drone->deliverySuccess = true;
			} else {
				dashboardMessage.state = D_DRONE_FLYING_CTM_DELIVERY_FAIL;
				mothershipMessage.type = DRONE_PACKAGE_DELIVERED_FAIL;
				drone->deliverySuccess = false;
				if(drone->package->numberOfTryRemaining > 1){
					clientMessage.type = DRONE_DELIVERY_FAILURE;
				}
				else{
					clientMessage.type = DRONE_DELIVERY_FINAL_FAILURE;
				}
			}
			pthread_mutex_unlock(&(drone->client->targetMutex));
			pthread_mutex_unlock(&(drone->mutex));

			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			mothership_sendMessage(drone->motherShip, &mothershipMessage);
			client_sendMessage(drone->client, &clientMessage);

			sleep(compute_sleep_time(drone));
			pthread_mutex_lock(&(drone->mutex));
			drone->autonomy -= consumption;
			pthread_mutex_unlock(&(drone->mutex));

			dashboardMessage.state = D_DRONE_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			mothershipMessage.type = DRONE_BACK_TO_MOTHERSHIP;
			mothership_sendMessage(drone->motherShip, &mothershipMessage);
			break;
		}
		default:
			break;
	}
	return true;
}

unsigned int compute_sleep_time(Drone* drone) {
	unsigned int distance = drone->client->distance - 1;
	// minus 1 to send the target message 1s before arriving

	return distance > 1 ? distance / 4 : 1;
}
