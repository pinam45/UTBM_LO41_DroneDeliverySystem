#include <stdlib.h>

#include "typedefs.h"
#include "package.h"

Package* package_constructor(unsigned int id, int priority, unsigned int weight, unsigned int clientID) {
	Package* package = (Package*)malloc(sizeof(Package));
	package->id = id;
	package->priority = priority;
	package->weight = weight;
	package->clientID = clientID;
	package->numberOfTryRemaining = 3; // TODO: Externalize

	return package;
}

int package_comparator(Package* lhs, Package* rhs) {
	return lhs->id - rhs->id;
}

void package_free(Package* package) {
	free(package);
}
