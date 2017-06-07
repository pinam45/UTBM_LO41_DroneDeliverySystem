#include "client.h"

#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include <client_message.h>
#include <log.h>
#include <pthread.h>

static unsigned int numberOfClient = 0;
static struct mq_attr attr;

static bool process_message(Client* client, ClientMessage* message);

Client* client_constructor(unsigned int distance) {
	Client* client = (Client*) malloc(sizeof(Client));
	client->id = numberOfClient++;
	client->distance = distance;
	client->targetInstalled = false;

	char buffer[11];
	sprintf(buffer, "/client%03d", client->id);

	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(ClientMessage);
	if((client->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)) == -1) {
		char errorBuffer[30];
		sprintf(errorBuffer, "Could not create client %03d", client->id);
		perror(errorBuffer);

		free(client);
		return NULL;
	}

	if(mq_unlink(buffer) == -1) {
		mq_close(client->msgQueueID);
		free(client);

		return NULL;
	}

	return client;
}

void client_free(Client* client) {
	if(mq_close(client->msgQueueID) == -1) {
		char errorBuffer[30];
		sprintf(errorBuffer, "Could not destroy client %03d", client->id);
		perror(errorBuffer);
	}

	free(client);
}

void* client_launch(void* client) {
	Client* client1 = (Client*) client;
	bool ended = false;
	while(!ended){
		ClientMessage clientMessage;
		check((int) mq_receive(client1->msgQueueID, (char*) &clientMessage, sizeof(ClientMessage), 0),
		      "mq_receive failed");
		ended = process_message(client1, &clientMessage);
	}
	pthread_exit(0);
}

void client_sendMessage(Client* client, ClientMessage* message) {
	check(mq_send(client->msgQueueID, (const char*) message, sizeof(ClientMessage), 0), "mq_send failed");
}

bool process_message(Client* client, ClientMessage* message) {
	switch(message->type){
		case DRONE_PUT_TARGET:
			client->targetInstalled = true;
			LOG_INFO("Client %03d target installed", client->id);
			break;
		case DRONE_DELIVERY_SUCCESS:
			client->targetInstalled = false;
			LOG_INFO("Client %03d package received", client->id);
			return true;
		case DRONE_DELIVERY_FAILURE:
			client->targetInstalled = false;
			LOG_INFO("Client %03d package not received", client->id);
			break;
		default:
			break;
	}
	return false;
}
