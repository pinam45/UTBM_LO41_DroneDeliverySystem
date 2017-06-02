#include "parser.h"

#include "package.h"
#include "drone.h"


LinkedList* loadDronesFromFile(FILE* file, Mothership* ship) {
	LinkedList* list = ll_createList();

	unsigned int maxLoad;
	unsigned int autonomy;
	unsigned int rechargingTime;

	while (fscanf(file, "%u,%u,%u\n", &maxLoad, &autonomy, &rechargingTime) != EOF) {
		ll_insertLast(list, drone_constructor(maxLoad, autonomy, rechargingTime, ship));
	}

	return list;
}

LinkedList* loadPackageFromFile(FILE* file) {
	LinkedList* list = ll_createList();

	int priority;
	unsigned int weight;

	while (fscanf(file, "%d,%u\n", &priority, &weight) != EOF) {
		Package* package = (Package*)malloc(sizeof(Package));
		package->priority = priority;
		package->weight = weight;

		ll_insertSorted(list, package,  (int(*)(void*, void*))&package_comparator);
	}

	return list;
}
