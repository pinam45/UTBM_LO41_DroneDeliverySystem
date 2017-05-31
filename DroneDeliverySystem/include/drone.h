#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H

#include <mqueue.h>

#include "package.h"
#include "typedefs.h"

/**
 * @brief Represents a drone
 */
struct drone {
	unsigned int id;
	unsigned int maxLoad; //!< max load of the drone.
	unsigned int autonomy; //!< autonomy in minutes.
	unsigned int rechargingTime; //!< recharging time in minutes.
	mqd_t msgQueueID;
	Mothership* motherShip;
	Client* client;
};

Drone* drone_constructor(unsigned int maxLoad, unsigned int autonomy, unsigned int rechargingTime, Mothership* motherShip);

void drone_free(Drone* drone);

/**
 * Computes the power consumption of the drone.
 * @param[in] drone a drone
 * @param[in] package a package that will be moved by the drone
 * @param[in] mothershipToClientDistance distance between the mothership and the client
 *
 * @return the power consumption of the drone
 */
unsigned int computePowerConsumption(Drone drone, Package package, double mothershipToClientDistance);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H
