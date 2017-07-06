#include <stdio.h>
#include <signal.h>

#include "typedefs.h"
#include "dashboard.h"
#include "drone.h"
#include "mothership.h"
#include "parser.h"
#include "util.h"

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

static void signalHandler(int signal);

void signalHandler(int UNUSED(signal)) {
	cc_Vector2 pos = {0, cc_getHeight() - 1};
	cc_setCursorPosition(pos);
	cc_setColors(BLACK, WHITE);
	cc_setCursorVisibility(true);
	cc_displayInputs(true);
	exit(130);
}

int main(int argc, char* argv[]) {
	srand(0);
	signal(SIGINT, &signalHandler);

	char files[3][255];

	if (argc == 1) {
		strcpy(files[0], "packages1.csv");
		strcpy(files[1], "clients1.csv");
		strcpy(files[2], "drones1.csv");
	} else if (argc == 4) {
		strcpy(files[0], argv[1]);
		strcpy(files[1], argv[2]);
		strcpy(files[2], argv[3]);
	} else {
		printf("Please read the README");
	}

	FILE* packageFile = fopen(files[0], "r");
	if(packageFile == NULL) {
		printf("ERROR: Unable to find packages1.csv\n");
		return EXIT_FAILURE;
	}
	LinkedList* packageList = loadPackagesFromFile(packageFile);
	fclose(packageFile);

	FILE* clientsFile = fopen(files[1], "r");
	if(clientsFile == NULL) {
		printf("ERROR: Unable to find clients1.csv\n");
		return EXIT_FAILURE;
	}
	LinkedList* clientList = loadClientsFromFile(clientsFile, packageList);
	fclose(clientsFile);

	FILE* droneFile = fopen(files[2], "r");
	if(droneFile == NULL) {
		printf("ERROR: Unable to find drones1.csv\n");
		return EXIT_FAILURE;
	}
	LinkedList* droneList = loadDronesFromFile(droneFile);
	fclose(droneFile);

	Mothership* mothership = mothership_constructor(droneList, clientList, packageList);

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

	mothership_free(mothership);
	dashboard_free(global_dashboard);

	pthread_exit(0);
}
