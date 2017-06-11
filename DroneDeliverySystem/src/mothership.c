#include <unistd.h>

#include "mothership.h"

#include "util.h"
#include "client.h"
#include "drone.h"
#include "LinkedList.h"
#include "client_message.h"
#include "drone_message.h"
#include "mothership_message.h"
#include "dashboard.h"

static struct mq_attr attr;

static int check_drone(void* droneId, void* drone);

int check_client(void* clientId, void* client);

static void process_message(Mothership* mothership, MothershipMessage* message);

static void find_package(Mothership* mothership, Drone* drone);

static void poweroff_drone(Drone* drone);

static bool has_at_least_one_package_valid(Mothership* mothership);

static void removeAvailable(Mothership* mothership, Drone* drone);

static void insertAvailable(Mothership* mothership, Drone* drone);

int check_drone(void* droneId, void* drone) {
	return *((unsigned int*) droneId) == ((Drone*) drone)->id;
}

int check_client(void* clientId, void* client) {
	return *((unsigned int*) clientId) == ((Client*) client)->id;
}

Mothership* mothership_constructor(LinkedList* droneList, LinkedList* clientList, LinkedList* packageList) {
	Mothership* mothership = (Mothership*) malloc(sizeof(Mothership));

	mothership->droneList = droneList;
	mothership->availableDrones = ll_createList(NULL);
	mothership->clientList = clientList;
	mothership->packageList = packageList;
	mothership->numberOfPackages = ll_getSize(packageList);

	const char* buffer = "/mothership";

	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(MothershipMessage);
	if((mothership->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)) == -1) {
		const char* errorBuffer = "Could not create mothership";
		perror(errorBuffer);

		ll_deleteList(droneList);
		ll_deleteList(clientList);
		ll_deleteList(packageList);
		free(mothership);
		return NULL;
	}

	if(mq_unlink(buffer) == -1) {
		mq_close(mothership->msgQueueID);
		free(mothership);

		return NULL;
	}

	return mothership;
}

void mothership_free(Mothership* mothership) {
	if(mq_close(mothership->msgQueueID) == -1) {
		const char* errorBuffer = "Could not destroy mothership";
		perror(errorBuffer);
	}

	ll_deleteList(mothership->droneList);
	ll_deleteList(mothership->packageList);
	ll_deleteList(mothership->clientList);
	ll_deleteListNoClean(mothership->availableDrones);

	free(mothership);
}

void mothership_launch(Mothership* mothership) {
	LinkedListIterator* listIterator;

	// Set packages as waiting in the dashboard
	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_PACKAGE;
	dashboardMessage.state = D_PACKAGE_WAITING;
	LinkedListIterator* packagesListIterator = ll_firstIterator(mothership->packageList);
	while(ll_hasNext(packagesListIterator)){
		Package* pkg = (Package*)ll_next(packagesListIterator);
		dashboardMessage.number = pkg->id;
		dashboard_sendMessage(global_dashboard, &dashboardMessage);
	}
	ll_deleteIterator(packagesListIterator);

	// Launch clients
	unsigned int clientsNumbers = ll_getSize(mothership->clientList);
	pthread_t clientsThreads[clientsNumbers];
	listIterator = ll_firstIterator(mothership->clientList);
	for(unsigned int i = 0; i < clientsNumbers; ++i) {
		Client* client = ll_next(listIterator);
		check(pthread_create(&clientsThreads[i], NULL, (void* (*)(void*)) &client_launch, (void*) client),
		      "pthread_create failed");
	}
	ll_deleteIterator(listIterator);

	// Launch drones
	unsigned int dronesNumbers = ll_getSize(mothership->droneList);
	pthread_t dronesThreads[dronesNumbers];
	listIterator = ll_firstIterator(mothership->droneList);
	for(unsigned int i = 0; i < dronesNumbers; ++i) {
		Drone* drone = ll_next(listIterator);
		check(pthread_create(&dronesThreads[i], NULL, (void* (*)(void*)) &drone_launch, (void*) drone),
		      "pthread_create failed");
	}
	ll_deleteIterator(listIterator);

	// Send drones deliver
	listIterator = ll_firstIterator(mothership->droneList);
	while(ll_hasNext(listIterator)) {
		find_package(mothership, (Drone*) ll_next(listIterator));
	}
	ll_deleteIterator(listIterator);

	// Main loop
	while(ll_getSize(mothership->droneList) != ll_getSize(mothership->availableDrones)) {
		LOG_INFO("[Mothership] %d/%d drones available", ll_getSize(mothership->availableDrones),
		         ll_getSize(mothership->droneList));
		MothershipMessage msg;
		mq_receive(mothership->msgQueueID, (char*) &msg, sizeof(MothershipMessage), 0);
		process_message(mothership, &msg);
	}

	// Power off drone
	listIterator = ll_firstIterator(mothership->droneList);
	while(ll_hasNext(listIterator)) {
		Drone* drone = ll_next(listIterator);
		if(drone->state == S_IN_MOTHERSHIP) {
			poweroff_drone(drone);
		}
	}
	ll_deleteIterator(listIterator);

	// Send failure to clients (if relevant)
	if (!ll_isEmpty(mothership->packageList)) {
		LinkedListIterator* it = ll_firstIterator(mothership->packageList);

		while (ll_hasNext(it)) {
			Package* pkg = ll_next(it);
			if (pkg->numberOfTryRemaining > 0) {
				ClientMessage msg;
				msg.type = MOTHERSHIP_UNABLE_TO_SEND_PACKAGE;
				DashboardMessage d_msg;
				d_msg.type = D_PACKAGE;
				d_msg.state = D_PACKAGE_FAIL;
				d_msg.number = pkg->id;

				client_sendMessage(ll_getElement(mothership->clientList, pkg->clientID), &msg);
				dashboard_sendMessage(global_dashboard, &d_msg);
			}
		}

		ll_deleteIterator(it);
	}
	// Join clients threads
	for(unsigned int i = 0; i < clientsNumbers; ++i) {
		check(pthread_join(clientsThreads[i], NULL), "pthread_join failed");
		LOG_INFO("Thread %d stopped", i);
	}

	// Join drones threads
	for(unsigned int i = 0; i < dronesNumbers; ++i) {
		check(pthread_join(dronesThreads[i], NULL), "pthread_join failed");
		LOG_INFO("Thread %d stopped", i);
	}
}

bool has_at_least_one_package_valid(Mothership* mothership) {
	if(mothership->numberOfPackages == 0) {
		return false;
	}

	if(ll_isEmpty(mothership->packageList)) {
		return true;
	}

	LinkedListIterator* it = ll_firstIterator(mothership->packageList);

	while(ll_hasNext(it)) {
		Package* package = (Package*) ll_next(it);
		if(package->numberOfTryRemaining > 0) {
			ll_deleteIterator(it);
			return true;
		}
	}

	ll_deleteIterator(it);
	return false;
}

void mothership_sendMessage(Mothership* mothership, MothershipMessage* message) {
	check(mq_send(mothership->msgQueueID, (const char*) message, sizeof(MothershipMessage), 0), "mq_send failed");
}

void process_message(Mothership* mothership, MothershipMessage* message) {
	LinkedListIterator* droneIt = ll_findElement(mothership->droneList, (void*) &message->sender_id, &check_drone);
	Drone* drone = (Drone*) ll_getValue(droneIt);

	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_PACKAGE;

	switch(message->type) {
		case DRONE_PACKAGE_DELIVERED_SUCCESS:
			dashboardMessage.number = drone->package->id;
			dashboardMessage.state = D_PACKAGE_SUCCESS;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			LOG_INFO("[Mothership] Drone %03d succeed", drone->id);
			free(drone->package);
			--(mothership->numberOfPackages);
			break;
		case DRONE_BACK_TO_MOTHERSHIP:
			LOG_INFO("[Mothership] Drone %03d back", drone->id);
			if(!drone->deliverySuccess) {
				dashboardMessage.number = drone->package->id;
				dashboardMessage.state = D_PACKAGE_WAITING;
				dashboard_sendMessage(global_dashboard, &dashboardMessage);

				--(drone->package->numberOfTryRemaining);
				ll_insertSorted(mothership->packageList, drone->package, (int (*)(void*, void*)) &package_comparator);
			}

			if(!ll_isEmpty(mothership->packageList)) {
				find_package(mothership, drone);
			} else if(mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
				insertAvailable(mothership, drone);
				poweroff_drone(drone);
			} else {
				insertAvailable(mothership, drone);
			}

			if(!drone->deliverySuccess && ll_getSize(mothership->availableDrones) > 0) {
				Drone* first_drone_available = (Drone*) ll_getFirst(mothership->availableDrones);
				removeAvailable(mothership, first_drone_available);
				find_package(mothership, first_drone_available);
			}

			break;
		case DRONE_DONE_CHARGING:
			if(ll_isEmpty(mothership->packageList)) {
				insertAvailable(mothership, drone);
				poweroff_drone(drone);
			} else {
				find_package(mothership, drone);
			}
			break;
		case DRONE_PACKAGE_DELIVERED_FAIL:
			dashboardMessage.number = drone->package->id;
			dashboardMessage.state = D_PACKAGE_FAIL;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("[Mothership] Drone %03d failed", drone->id);
			break;
		case DRONE_DEAD:
			ll_removeIt(droneIt);
			break;
		default:
			break;
	}

	ll_deleteIterator(droneIt);
}

void poweroff_drone(Drone* drone) {
	DroneMessage answer;
	answer.type = MOTHERSHIP_END_OF_DELIVERY;
	drone_sendMessage(drone, &answer);
}

void find_package(Mothership* mothership, Drone* drone) {
	Package* pkg = NULL;
	Client* client = NULL;

	if(!ll_isEmpty(mothership->packageList)) {
		LinkedListIterator* it = ll_firstIterator(mothership->packageList);

		bool battery = false;
		while(ll_hasNext(it) && pkg == NULL) {
			Package* value = (Package*) ll_next(it);
			LinkedListIterator* clientIt = ll_findElement(mothership->clientList, (void*) &(value->clientID), &check_client);
			Client* tmpClient = (Client*) ll_getValue(clientIt);
			ll_deleteIterator(clientIt);

			battery = computePowerConsumption(drone, value, tmpClient->distance) < drone->autonomy;

			if(value->weight <= drone->maxLoad && value->numberOfTryRemaining > 0 && drone->package != value &&
			   battery) {
				pkg = value;
				client = tmpClient;
			}
		}

		if(pkg != NULL) {
			DashboardMessage dashboardMessage;
			dashboardMessage.type = D_PACKAGE;
			dashboardMessage.number = pkg->id;
			dashboardMessage.state = D_PACKAGE_FLYING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			(void) ll_prev(it);
			ll_removeIt(it);

			drone->package = pkg;
			drone->client = client;
			removeAvailable(mothership, drone);

			DroneMessage answer;
			answer.type = MOTHERSHIP_GO_DELIVER_PACKAGE;
			drone_sendMessage(drone, &answer);
		}
		else if(mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
			insertAvailable(mothership, drone);
			poweroff_drone(drone);
		}
		else if(!battery && drone->maxAutonomy != drone->autonomy) {
			DroneMessage answer;
			answer.type = MOTHERSHIP_GO_RECHARGE;

			drone->package = NULL;
			drone_sendMessage(drone, &answer);
		}
		else {
			insertAvailable(mothership, drone);
		}

		ll_deleteIterator(it);
	} else if(mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
		insertAvailable(mothership, drone);
		poweroff_drone(drone);
	} else {
		insertAvailable(mothership, drone);
	}
}

void removeAvailable(Mothership* mothership, Drone* drone) {
	LinkedListIterator* itA = ll_findElement(mothership->availableDrones, drone, &check_drone);
	if(itA != NULL) {
		LOG_INFO("[Mothership] Drone %03d is not available", drone->id);
		ll_removeIt(itA);
	}

	ll_deleteIterator(itA);
}

void insertAvailable(Mothership* mothership, Drone* drone) {
	LOG_INFO("[Mothership] Drone %03d is available", drone->id);

	ll_insertLast(mothership->availableDrones, drone);
}
