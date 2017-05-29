#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_DRONE_H

#include <package.h>

/**
 * @brief Represents a drone
 */
typedef struct {
	unsigned int maxLoad; //!< max load of the drone.
	unsigned int autonomy; //!< autonomy in minutes.
	unsigned int rechargingTime; //!< recharging time in minutes.
} Drone;

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
