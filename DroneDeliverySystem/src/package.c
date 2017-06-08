#include <stdlib.h>

#include "typedefs.h"
#include "package.h"

Package* package_constructor(int priority, unsigned int weight, unsigned int clientID) {
	Package* package = (Package*)malloc(sizeof(Package));

	package->priority = priority;
	package->weight = weight;
	package->clientID = clientID;
	package->numberOfTryRemaining = 3; // TODO: Externalize

	return package;
}

int package_comparator(Package* lhs, Package* rhs) {
	if (lhs->priority == rhs->priority) {
		return (int)lhs->weight - (int)rhs->weight;
	}

	return lhs->priority - rhs->priority;
}

void package_free(Package* package) {
	free(package);
}
