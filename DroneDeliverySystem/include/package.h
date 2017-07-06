#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H

#include "typedefs.h"

/**
 * @brief Represents a package.
 */
struct package {
	unsigned int id;
	int priority; //!< priority of a package
	unsigned int weight; //!< weight of a package
	unsigned int clientID; //!< id of the client.
	unsigned int numberOfTryRemaining;
};

Package* package_constructor(unsigned int id, int priority, unsigned int weight, unsigned int clientID, unsigned int numberOfTryRemaining);

int package_comparator(Package* lhs, Package* rhs);

void package_free(Package* pkg);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H
