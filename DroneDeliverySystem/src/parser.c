#include "parser.h"

#include "package.h"
#include "drone.h"
#include "client.h"

LinkedList* loadDronesFromFile(FILE* file, Mothership* ship) {
	LinkedList* list = ll_createList((void(*)(void*))&drone_free);

	unsigned int id;
	unsigned int maxLoad;
	unsigned int autonomy;
	unsigned int rechargingTime;

	while(fscanf(file, "%u,%u,%u,%u\n", &id, &maxLoad, &autonomy, &rechargingTime) != EOF) {
		ll_insertLast(list, drone_constructor(id, maxLoad, autonomy, rechargingTime, ship));
	}

	return list;
}

LinkedList* loadPackagesFromFile(FILE* file) {
	LinkedList* list = ll_createList((void(*)(void*))&package_free);

	unsigned int id;
	int priority;
	unsigned int weight;
	unsigned int clientID;
	unsigned int numberOfTryRemaining;

	while(fscanf(file, "%u,%d,%u,%u,%u\n", &id, &priority, &weight, &clientID, &numberOfTryRemaining) != EOF) {
		Package* package = package_constructor(id, priority, weight, clientID, numberOfTryRemaining);

		ll_insertSorted(list, package, (int (*)(void*, void*)) &package_comparator);
	}

	return list;
}

LinkedList* loadClientsFromFile(FILE* file) {
	LinkedList* list = ll_createList((void(*)(void*))&client_free);

	unsigned int id;
	unsigned int distance;
	unsigned int packagesToReceive;

	while(fscanf(file, "%u,%u,%u\n", &id, &distance, &packagesToReceive) != EOF) {
		ll_insertLast(list, client_constructor(id, distance, packagesToReceive));
	}

	return list;
}
