#include <stdio.h>
#include <dashboard.h>
#include <util.h>

#include "typedefs.h"
#include "drone.h"
#include "mothership.h"
#include "parser.h"

int main() {
	//FIXME: test start
	LinkedList* droneList = ll_createList();
	LinkedList* clientList = ll_createList();
	FILE* packageFile = fopen("Packages.txt", "r");
	if(packageFile == NULL) {
		SLOG_ERR("Unable to find Packages.txt");
	}

	LinkedList* packageList = loadPackagesFromFile(packageFile);
	fclose(packageFile);
	Mothership* mothership = mothership_constructor(droneList, clientList, packageList);
	if(mothership == NULL) {
		printf("aaaa");
	}

	Drone* drone1 = drone_constructor(0, 10, 3, 2, mothership);
	ll_insertLast(droneList, drone1);
	Drone* drone2 = drone_constructor(1, 10, 4, 2, mothership);
	ll_insertLast(droneList, drone2);
	Drone* drone3 = drone_constructor(2, 10, 10, 2, mothership);
	ll_insertLast(droneList, drone3);
	Drone* drone4 = drone_constructor(3, 10, 10, 2, mothership);
	ll_insertLast(droneList, drone4);
	Drone* drone5 = drone_constructor(4, 10, 6, 2, mothership);
	ll_insertLast(droneList, drone5);

	global_dashboard = dashboard_constructor(
		ll_getSize(packageList),
		ll_getSize(droneList),
		ll_getSize(clientList)
	);
	pthread_t dashboardThread;
	check(pthread_create(&dashboardThread, NULL, &dashboard_launch, (void*) global_dashboard), "pthread_create failed");

	mothership_launch(mothership);
	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_EXIT;
	dashboard_sendMessage(global_dashboard, &dashboardMessage);

	//FIXME: test end

	mothership_free(mothership);
	dashboard_free(global_dashboard);

	pthread_exit(0);
}
