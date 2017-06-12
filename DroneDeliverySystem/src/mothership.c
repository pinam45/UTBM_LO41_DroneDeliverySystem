#include <unistd.h>
#include <LinkedList.h>

#include "mothership.h"

#include "util.h"
#include "client.h"
#include "drone.h"
#include "LinkedList.h"
#include "client_message.h"
#include "drone_message.h"
#include "mothership_message.h"
#include "dashboard.h"

static int check_drone(void* droneId, void* drone);

int check_client(void* clientId, void* client);

static void process_message(Mothership* mothership, MothershipMessage* message);

static bool find_package(Mothership* mothership, Drone* drone);

static void poweroff_drone(Drone* drone);

static bool has_at_least_one_package_valid(Mothership* mothership);

static void removeAvailable(Mothership* mothership, Drone* drone);

static void insertAvailable(Mothership* mothership, Drone* drone);

void assign_drones(Mothership* mothership);

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
	mothership->deadDronesNbr = 0;

	const char* buffer = "/mothership";
	struct mq_attr attr;
	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(MothershipMessage);
	check((mothership->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)), "Mothership: mq_open failed");
	check(mq_unlink(buffer), "Mothership: mq_unlink failed");

	return mothership;
}

void mothership_free(Mothership* mothership) {
	ll_deleteList(mothership->droneList);
	ll_deleteList(mothership->packageList);
	ll_deleteList(mothership->clientList);
	ll_deleteListNoClean(mothership->availableDrones);
	check(mq_close(mothership->msgQueueID), "Mothership: mq_close failed");
	free(mothership);
}

void mothership_launch(Mothership* mothership) {
	LinkedListIterator* listIterator;

	// Set packages as waiting in the dashboard
	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_PACKAGE;
	dashboardMessage.state = D_PACKAGE_WAITING;
	LinkedListIterator* packagesListIterator = ll_firstIterator(mothership->packageList);
	while (ll_hasNext(packagesListIterator)) {
		Package* pkg = (Package*) ll_next(packagesListIterator);
		dashboardMessage.number = pkg->id;
		dashboard_sendMessage(global_dashboard, &dashboardMessage);
	}
	ll_deleteIterator(packagesListIterator);

	// Launch clients
	unsigned int clientsNumbers = ll_getSize(mothership->clientList);
	pthread_t clientsThreads[clientsNumbers];
	listIterator = ll_firstIterator(mothership->clientList);
	for (unsigned int i = 0; i < clientsNumbers; ++i) {
		Client* client = ll_next(listIterator);
		check(pthread_create(&clientsThreads[i], NULL, (void* (*)(void*)) &client_launch, (void*) client),
		      "pthread_create failed");
	}
	ll_deleteIterator(listIterator);

	// Launch drones
	unsigned int dronesNumbers = ll_getSize(mothership->droneList);
	pthread_t dronesThreads[dronesNumbers];
	listIterator = ll_firstIterator(mothership->droneList);
	for (unsigned int i = 0; i < dronesNumbers; ++i) {
		Drone* drone = ll_next(listIterator);
		drone->motherShip = mothership;
		check(pthread_create(&dronesThreads[i], NULL, (void* (*)(void*)) &drone_launch, (void*) drone),
		      "pthread_create failed");
	}
	ll_deleteIterator(listIterator);

	// Send drones deliver
	listIterator = ll_firstIterator(mothership->droneList);
	while (ll_hasNext(listIterator)) {
		insertAvailable(mothership, (Drone*) ll_next(listIterator));
	}
	ll_deleteIterator(listIterator);

	assign_drones(mothership);

	// Main loop
	while (ll_getSize(mothership->droneList) != ll_getSize(mothership->availableDrones) + mothership->deadDronesNbr) {
		MothershipMessage msg;
		mq_receive(mothership->msgQueueID, (char*) &msg, sizeof(MothershipMessage), 0);
		process_message(mothership, &msg);

		assign_drones(mothership);
	}

	// Power off drone
	listIterator = ll_firstIterator(mothership->droneList);
	while (ll_hasNext(listIterator)) {
		Drone* drone = ll_next(listIterator);
		pthread_mutex_lock(&(drone->mutex));
		if (drone->state == S_ALIVE) {
			pthread_mutex_unlock(&(drone->mutex));
			poweroff_drone(drone);
		}
		else{
			pthread_mutex_unlock(&(drone->mutex));
		}
	}
	ll_deleteIterator(listIterator);

	// Join clients threads
	for (unsigned int i = 0; i < clientsNumbers; ++i) {
		check(pthread_join(clientsThreads[i], NULL), "pthread_join failed");
		LOG_INFO("Thread %d stopped", i);
	}

	// Join drones threads
	for (unsigned int i = 0; i < dronesNumbers; ++i) {
		check(pthread_join(dronesThreads[i], NULL), "pthread_join failed");
		LOG_INFO("Thread %d stopped", i);
	}
}

void assign_drones(Mothership* mothership) {
	if(ll_getSize(mothership->availableDrones) > 0 && has_at_least_one_package_valid(mothership)) {
		while (!ll_isEmpty(mothership->availableDrones) && !ll_isEmpty(mothership->packageList)) {
			Drone* first_drone_available = (Drone*) ll_getFirst(mothership->availableDrones);
			removeAvailable(mothership, first_drone_available);
			find_package(mothership, first_drone_available);
		}
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
			pthread_mutex_lock(&(drone->mutex));
			dashboardMessage.number = drone->package->id;
			dashboardMessage.state = D_PACKAGE_SUCCESS;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);

			LOG_INFO("[Mothership] Drone %03d succeed", drone->id);
			free(drone->package);
			drone->package = NULL;
			pthread_mutex_unlock(&(drone->mutex));
			--(mothership->numberOfPackages);
			break;
		case DRONE_BACK_TO_MOTHERSHIP:
			LOG_INFO("[Mothership] Drone %03d back", drone->id);
			pthread_mutex_lock(&(drone->mutex));
			if(!drone->deliverySuccess) {
				dashboardMessage.number = drone->package->id;

				if (drone->package->numberOfTryRemaining > 0) {
					ll_insertSorted(mothership->packageList, drone->package,
					                (int (*)(void*, void*)) &package_comparator);
					drone->package = NULL;
					dashboardMessage.state = D_PACKAGE_WAITING;
				} else {
					package_free(drone->package);
					drone->package = NULL;
					--(mothership->numberOfPackages);
					dashboardMessage.state = D_PACKAGE_FAIL;
				}

				dashboard_sendMessage(global_dashboard, &dashboardMessage);
			}

			pthread_mutex_unlock(&(drone->mutex));

			insertAvailable(mothership, drone);

			break;
		case DRONE_DONE_CHARGING:
			insertAvailable(mothership, drone);
			break;
		case DRONE_PACKAGE_DELIVERED_FAIL:
			pthread_mutex_lock(&(drone->mutex));
			dashboardMessage.number = drone->package->id;
			pthread_mutex_unlock(&(drone->mutex));
			dashboardMessage.state = D_PACKAGE_FAIL;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("[Mothership] Drone %03d failed", drone->id);

			pthread_mutex_lock(&(drone->mutex));
			if (drone->package->numberOfTryRemaining > 0) {
				--(drone->package->numberOfTryRemaining);
			}
			pthread_mutex_unlock(&(drone->mutex));

			break;
		case DRONE_DEAD: {
			Client* client = NULL;
			pthread_mutex_lock(&(drone->mutex));
			if (drone->package != NULL) {
				client = (Client*) ll_getElement(mothership->clientList, drone->package->clientID);
				--(mothership->numberOfPackages);
				package_free(drone->package);
				drone->package = NULL;
			}
			pthread_mutex_unlock(&(drone->mutex));
			if (client != NULL) {
				ClientMessage clientMessage;
				clientMessage.type = DRONE_DELIVERY_FINAL_FAILURE;
				client_sendMessage(client, &clientMessage);
			}
			++(mothership->deadDronesNbr);
		}
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

bool find_package(Mothership* mothership, Drone* drone) {
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

			pthread_mutex_lock(&(drone->mutex));
			battery = computePowerConsumption(value, tmpClient->distance) < drone->autonomy;

			if(value->weight <= drone->maxLoad && value->numberOfTryRemaining != 0 && battery) {
				pkg = value;
				client = tmpClient;
			}
			pthread_mutex_unlock(&(drone->mutex));
		}

		pthread_mutex_lock(&(drone->mutex));
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
		} else if(mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
			insertAvailable(mothership, drone);
			poweroff_drone(drone);
		} else if(!battery && drone->maxAutonomy != drone->autonomy) {
			DroneMessage answer;
			answer.type = MOTHERSHIP_GO_RECHARGE;

			drone->package = NULL;
			drone_sendMessage(drone, &answer);
		} else {
			insertAvailable(mothership, drone);
		}
		pthread_mutex_unlock(&(drone->mutex));

		ll_deleteIterator(it);
	} else if(mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
		insertAvailable(mothership, drone);
		poweroff_drone(drone);
	} else {
		insertAvailable(mothership, drone);
	}

	return pkg != NULL;
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
