#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H

#include "typedefs.h"

/**
 * @brief Represents a package.
 */
struct package {
	int priority; //!< priority of a package
	unsigned int weight; //!< weight of a package
	unsigned int clientID; //!< id of the client.
	unsigned int numberOfTryRemaining;
};

Package* package_constructor(int priority, unsigned int weight, unsigned int clientID);

int package_comparator(Package* lhs, Package* rhs);

void package_destructor(Package* pkg);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H
