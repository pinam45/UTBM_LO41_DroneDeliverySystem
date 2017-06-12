#ifndef UTBM_LO41_DRONEDELIVERYSYSTEM_PARSER_H
#define UTBM_LO41_DRONEDELIVERYSYSTEM_PARSER_H

#include "typedefs.h"
#include "LinkedList.h"

LinkedList* loadDronesFromFile(FILE* file);

LinkedList* loadPackagesFromFile(FILE* file);

LinkedList* loadClientsFromFile(FILE* file);

#endif //UTBM_LO41_DRONEDELIVERYSYSTEM_PARSER_H
