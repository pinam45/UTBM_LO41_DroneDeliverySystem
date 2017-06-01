#include "mothership.h"

#include <util.h>
#include <unistd.h>  //FIXME: sleep for test

#include "drone.h"
#include "drone_message.hpp"
#include "mothership_message.h"

static struct mq_attr attr;

static int check_drone(void* droneId, void* drone);

int check_drone(void* droneId, void* drone) {
	return *((unsigned int*) droneId) == ((Drone*) drone)->id;
}

Mothership* mothership_constructor(LinkedList* droneList, LinkedList* clientList, LinkedList* packageList) {
	Mothership* mothership = (Mothership*) malloc(sizeof(Mothership));

	mothership->droneList = droneList;
	mothership->clientList = clientList;
	mothership->packageList = packageList;

	const char* buffer = "/mothership";

	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(MothershipMessage);
	if((mothership->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)) == -1) {
		const char* errorBuffer = "Could not create mothership";
		perror(errorBuffer);

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

	free(mothership);
}

void mothership_launch(Mothership* mothership) {
	unsigned int dronesNumbers = ll_getSize(mothership->droneList);

	pthread_attr_t attrs[dronesNumbers];
	pthread_t threads[dronesNumbers];
	LinkedListIterator* listIterator = ll_firstIterator(mothership->droneList);
	for(unsigned int i = 0; i < dronesNumbers; ++i) {
		Drone* drone = ll_next(listIterator);
		check(pthread_attr_init(&attrs[i]), "pthread_attr_init failed");
		check(pthread_attr_setdetachstate(&attrs[i], PTHREAD_CREATE_DETACHED), "pthread_attr_setdetachstate failed");
		check(pthread_create(&threads[i], &attrs[i], &drone_launch, (void*) drone), "pthread_create failed");
	}

	//FIXME: test start
	while(true) {
		MothershipMessage mothershipMessage;
		check((int) mq_receive(mothership->msgQueueID, (char*) &mothershipMessage, sizeof(MothershipMessage), 0),
		      "mq_receive failed");
		printf("[Mothership] Received message from drone %d\n", mothershipMessage.sender_id);

		Drone* drone = ll_findElement(mothership->droneList, (void*) &mothershipMessage.sender_id, &check_drone);
		if(drone != NULL) {
			DroneMessage droneMessage;
			droneMessage.type = MOTHERSHIP_GO_DELIVER_PACKAGE;
			drone_sendMessage(drone, &droneMessage);
			printf("[Mothership] Sent message to drone %d\n", drone->id);
		} else {
			printf("[Mothership] Drone %d not found", mothershipMessage.sender_id);
		}
	}
	//FIXME: test end

	for(unsigned int i = 0; i < dronesNumbers; ++i) {
		check(pthread_attr_destroy(&attrs[i]), "pthread_attr_destroy failed");
	}
}

void mothership_sendMessage(Mothership* mothership, MothershipMessage* message) {
	check(mq_send(mothership->msgQueueID, (const char*) message, sizeof(MothershipMessage), 0), "mq_send failed");
}
