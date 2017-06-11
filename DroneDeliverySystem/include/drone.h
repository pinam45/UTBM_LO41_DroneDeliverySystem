#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H

#include <mqueue.h>
#include <stdbool.h>

#include "package.h"
#include "typedefs.h"


typedef enum {
	S_ALIVE,
	S_DEAD
} State;

/**
 * @brief Represents a drone
 */
struct drone {
	unsigned int id;
	unsigned int maxLoad; //!< max load of the drone.
	unsigned int autonomy; //!< autonomy in minutes.
	unsigned int maxAutonomy;
	unsigned int rechargingTime; //!< recharging time in minutes.
	Mothership* motherShip;
	Client* client;
	Package* package;
	bool deliverySuccess;
	State state;
	mqd_t msgQueueID;
	pthread_mutex_t mutex;
};

Drone* drone_constructor(unsigned int id, unsigned int maxLoad, unsigned int autonomy, unsigned int rechargingTime, Mothership* motherShip);

void drone_free(Drone* drone);

void* drone_launch(Drone* drone);

void drone_sendMessage(Drone* drone, DroneMessage* message);

/**
 * Computes the power consumption of the drone.
 * @param[in] drone a drone
 * @param[in] package a package that will be moved by the drone
 * @param[in] mothershipToClientDistance distance between the mothership and the client
 *
 * @return the power consumption of the drone
 */
unsigned int computePowerConsumption(Drone* drone, Package* package, double mothershipToClientDistance);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H
