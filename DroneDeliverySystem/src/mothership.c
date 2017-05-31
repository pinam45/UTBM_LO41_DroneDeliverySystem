#include "mothership.h"

static struct mq_attr attr = { 0, 20, 2048, 0};

Mothership* mothership_constructor(LinkedList* droneList, LinkedList* clientList, LinkedList* packageList) {
	Mothership* mothership = (Mothership*)malloc(sizeof(Mothership));

	mothership->droneList = droneList;
	mothership->clientList = clientList;
	mothership->packageList = packageList;

	const char* buffer = "/mothership";

	if ((mothership->msgQueueID = mq_open(buffer, O_WRONLY | O_CREAT, 0660, &attr)) == -1) {
		const char* errorBuffer = "Could not create mothership";
		perror(errorBuffer);

		free(mothership);
		return NULL;
	}

	if (mq_unlink(buffer) == -1) {
		mq_close(mothership->msgQueueID);
		free(mothership);

		return NULL;
	}

	return mothership;
}

void mothership_free(Mothership* mothership) {
	if (mq_close(mothership->msgQueueID) == -1) {
		const char* errorBuffer = "Could not destroy mothership";
		perror(errorBuffer);
	}

	free(mothership);
}
