#include <stdio.h>
#include <dashboard.h>
#include <util.h>
#include <signal.h>

#include "typedefs.h"
#include "drone.h"
#include "mothership.h"
#include "parser.h"

static void signalHandler(int signal);

void signalHandler(int signal) {
	cc_setCursorVisibility(true);
	exit(130);
}

int main() {
	srand(0);
	signal(SIGINT, &signalHandler);

	//FIXME: test start
	LinkedList* droneList = ll_createList((void(*)(void*))&drone_free);

	FILE* clientsFile = fopen("Clients.txt", "r");
	if(clientsFile == NULL) {
		SLOG_ERR("Unable to find Clients.txt");
	}
	LinkedList* clientList = loadClientsFromFile(clientsFile);
	fclose(clientsFile);

	FILE* packageFile = fopen("Packages.txt", "r");
	if(packageFile == NULL) {
		SLOG_ERR("Unable to find Packages.txt");
	}
	LinkedList* packageList = loadPackagesFromFile(packageFile);
	fclose(packageFile);

	Mothership* mothership = mothership_constructor(droneList, clientList, packageList);

	Drone* drone1 = drone_constructor(0, 15, 19, 1, mothership);
	ll_insertLast(droneList, drone1);
	Drone* drone2 = drone_constructor(1, 16, 17, 4, mothership);
	ll_insertLast(droneList, drone2);
	Drone* drone3 = drone_constructor(2, 12, 20, 1, mothership);
	ll_insertLast(droneList, drone3);
	Drone* drone4 = drone_constructor(3, 20, 20, 3, mothership);
	ll_insertLast(droneList, drone4);
	Drone* drone5 = drone_constructor(4, 17, 18, 2, mothership);
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
	check(pthread_join(dashboardThread, NULL), "pthread_join failed");

	//FIXME: test end

	mothership_free(mothership);
	dashboard_free(global_dashboard);

	pthread_exit(0);
}
