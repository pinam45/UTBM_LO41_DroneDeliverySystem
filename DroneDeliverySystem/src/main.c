#include <stdio.h>
#include <typedefs.h>
#include "drone.h"
#include "mothership.h"

int main() {
	printf("Hello, World!");

	//FIXME: test start
	LinkedList* droneList = ll_createList();
	LinkedList* clientList = ll_createList();
	LinkedList* packageList = ll_createList();
	Mothership* mothership = mothership_constructor(droneList, clientList, packageList);
	if(mothership == NULL) {
		printf("aaaa");
	}

	Drone* drone1 = drone_constructor(0, 10, 10, 10, mothership);
	ll_insertLast(droneList, drone1);
	Drone* drone2 = drone_constructor(1, 10, 10, 10, mothership);
	ll_insertLast(droneList, drone2);
	Drone* drone3 = drone_constructor(2, 10, 10, 10, mothership);
	ll_insertLast(droneList, drone3);
	Drone* drone4 = drone_constructor(3, 10, 10, 10, mothership);
	ll_insertLast(droneList, drone4);
	Drone* drone5 = drone_constructor(4, 10, 10, 10, mothership);
	ll_insertLast(droneList, drone5);

	mothership_launch(mothership);
	//FIXME: test end

	return 0;
}
