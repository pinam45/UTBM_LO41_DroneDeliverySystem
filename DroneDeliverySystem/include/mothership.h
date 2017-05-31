#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_HPP
#define UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_HPP

#include "LinkedList.h"
#include "drone.h"
#include "typedefs.h"

struct mothership {
	LinkedList droneList;
};

// TODO
MotherShip* mothership_constructor();

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_MOTHERSHIP_HPP
