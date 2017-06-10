#include "mothership.h"

#include <util.h>
#include <unistd.h>  //FIXME: sleep for test
#include <LinkedList.h>
#include <client.h>

#include "drone.h"
#include "drone_message.h"
#include "mothership_message.h"

static struct mq_attr attr;

static int check_drone(void* droneId, void* drone);
static void process_message(Mothership* mothership, MothershipMessage* message);
static void find_package(Mothership* mothership, Drone* drone);
static void poweroff_drone(Drone* drone);
static bool has_at_least_one_package_valid(Mothership* mothership);
static void removeAvailable(Mothership* mothership, Drone* drone);
static void insertAvailable(Mothership* mothership, Drone* drone);

int check_drone(void* droneId, void* drone) {
	return *((unsigned int*) droneId) == ((Drone*) drone)->id;
}

Mothership* mothership_constructor(LinkedList* droneList, LinkedList* clientList, LinkedList* packageList) {
	Mothership* mothership = (Mothership*) malloc(sizeof(Mothership));

	mothership->droneList = droneList;
	mothership->availableDrones = ll_createList();
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
	unsigned int dronesNumbers = ll_getSize(mothership->droneList);

	pthread_t threads[dronesNumbers];
	LinkedListIterator* listIterator = ll_firstIterator(mothership->droneList);
	for(unsigned int i = 0; i < dronesNumbers; ++i) {
		Drone* drone = ll_next(listIterator);
		check(pthread_create(&threads[i], NULL, (void*(*)(void*))&drone_launch, (void*) drone), "pthread_create failed");
	}

	ll_deleteIterator(listIterator);
	listIterator = ll_firstIterator(mothership->droneList);

	while (ll_hasNext(listIterator)) {
		find_package(mothership, (Drone*)ll_next(listIterator));
	}

	ll_deleteIterator(listIterator);

	while (ll_getSize(mothership->droneList) != ll_getSize(mothership->availableDrones)) {
		LOG_INFO("[Mothership] %d/%d drones available", ll_getSize(mothership->availableDrones), ll_getSize(mothership->droneList));
		MothershipMessage msg;
		mq_receive(mothership->msgQueueID, (char*)&msg, sizeof(MothershipMessage), 0);
		process_message(mothership, &msg);
	}

	listIterator = ll_firstIterator(mothership->droneList);
	while (ll_hasNext(listIterator)) {
		Drone* drone = ll_next(listIterator);
		if (drone->state == S_IN_MOTHERSHIP) {
			poweroff_drone(drone);
		}
	}

	ll_deleteIterator(listIterator);

	for(unsigned int i = 0; i < dronesNumbers; ++i) {
		check(pthread_join(threads[i], NULL), "pthread_join failed");
		LOG_INFO("Thread %d stopped", i);
	}
}

bool has_at_least_one_package_valid(Mothership* mothership) {
	if (mothership->numberOfPackages == 0) {
		return false;
	}

	if (ll_isEmpty(mothership->packageList)) {
		return true;
	}

	LinkedListIterator* it = ll_firstIterator(mothership->packageList);

	while (ll_hasNext(it)) {
		Package* package = (Package*)ll_next(it);
		if (package->numberOfTryRemaining > 0) {
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
	Drone* drone = (Drone*)ll_getValue(droneIt);

	switch (message->type) {
		case DRONE_PACKAGE_DELIVERED_SUCCESS:
			LOG_INFO("[Mothership] Drone %03d succeed", drone->id);
			drone->deliverySucess = true;
			--(mothership->numberOfPackages);
			break;
		case DRONE_BACK_TO_MOTHERSHIP:
			LOG_INFO("[Mothership] Drone %03d back", drone->id);
			if (!drone->deliverySucess) {
				--(drone->package->numberOfTryRemaining);
				ll_insertSorted(mothership->packageList, drone->package, (int(*)(void*, void*))&package_comparator);
			}

			if (!ll_isEmpty(mothership->packageList)) {
				find_package(mothership, drone);
			} else if (mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
				insertAvailable(mothership, drone);
				poweroff_drone(drone);
			} else {
				insertAvailable(mothership, drone);
			}

			if (!drone->deliverySucess && ll_getSize(mothership->availableDrones) > 1) {
				Drone* first_drone_available = (Drone*)ll_getFirst(mothership->availableDrones);
				removeAvailable(mothership, first_drone_available);
				find_package(mothership, first_drone_available);
			}

			break;
		case DRONE_DONE_CHARGING:
			if (ll_isEmpty(mothership->packageList)) {
				poweroff_drone(drone);
			} else {
				find_package(mothership, drone);
			}
			break;
		case DRONE_PACKAGE_DELIVERED_FAIL:
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
	drone->state = S_DEAD;

	drone_sendMessage(drone, &answer);
}

void find_package(Mothership* mothership, Drone* drone) {
	Package* pkg = NULL;

	if (!ll_isEmpty(mothership->packageList)) {
		LinkedListIterator* it = ll_firstIterator(mothership->packageList);

		bool battery = false;
		while (ll_hasNext(it) && pkg == NULL) {
			Package* value = (Package*) ll_next(it);
			// FIXME
			//Client* client = (Client*) ll_getElement(mothership->clientList, value->clientID);
			battery = computePowerConsumption(drone, value, 5) < drone->autonomy;

			if (value->weight <= drone->maxLoad && value->numberOfTryRemaining > 0 && drone->package != value && battery) {
				pkg = value;
			}
		}

		if (pkg != NULL) {
			DroneMessage answer;
			answer.type = MOTHERSHIP_GO_DELIVER_PACKAGE;

			(void)ll_prev(it);
			ll_removeIt(it);

			drone->package = pkg;
			removeAvailable(mothership, drone);

			drone_sendMessage(drone, &answer);
		} else if (mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
			insertAvailable(mothership, drone);
			poweroff_drone(drone);
		} else if (!battery && drone->maxAutonomy != drone->autonomy) {
			DroneMessage answer;
			answer.type = MOTHERSHIP_GO_RECHARGE;

			drone->package = NULL;
			drone_sendMessage(drone, &answer);
		} else {
			insertAvailable(mothership, drone);
		}

		ll_deleteIterator(it);
	} else if (mothership->numberOfPackages == 0 || !has_at_least_one_package_valid(mothership)) {
	    insertAvailable(mothership, drone);
		poweroff_drone(drone);
	} else {
		insertAvailable(mothership, drone);
	}
}

void removeAvailable(Mothership* mothership, Drone* drone) {
	LinkedListIterator* itA = ll_findElement(mothership->availableDrones, drone, &check_drone);
	if (itA != NULL) {
		LOG_INFO("[Mothership] Drone %03d is not available", drone->id);
		ll_removeIt(itA);
	}

	ll_deleteIterator(itA);
}

void insertAvailable(Mothership* mothership, Drone* drone) {
	LOG_INFO("[Mothership] Drone %03d is available", drone->id);

	ll_insertLast(mothership->availableDrones, drone);
}
