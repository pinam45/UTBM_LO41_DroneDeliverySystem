#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H

#include "typedefs.h"

/**
 * @brief Represents a package.
 */
struct package {
	int priority; //!< priority of a package
	unsigned int weight; //!< weight of a package
};

int package_comparator(Package* lhs, Package* rhs);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_PACKAGE_H
