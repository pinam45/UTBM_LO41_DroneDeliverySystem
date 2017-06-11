#include <stdlib.h>

#include "typedefs.h"
#include "package.h"

Package* package_constructor(unsigned int id, int priority, unsigned int weight, unsigned int clientID, unsigned int numberOfTryRemaining) {
	Package* package = (Package*)malloc(sizeof(Package));
	package->id = id;
	package->priority = priority;
	package->weight = weight;
	package->clientID = clientID;
	package->numberOfTryRemaining = numberOfTryRemaining;

	return package;
}

int package_comparator(Package* lhs, Package* rhs) {
	return lhs->id - rhs->id;
}

void package_free(Package* package) {
	free(package);
}
