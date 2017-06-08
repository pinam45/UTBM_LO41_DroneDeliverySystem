#include "client.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <mqueue.h>

#include "util.h"
#include "client_message.h"
#include "log.h"

static struct mq_attr attr;

static void process_message(Client* client, ClientMessage* message);

Client* client_constructor(unsigned int id, unsigned int distance, unsigned int packagesToReceive, unsigned int targetInstalledTime) {
	Client* client = (Client*) malloc(sizeof(Client));
	client->id = id;
	client->distance = distance;
	client->packagesToReceive = packagesToReceive;
	client->targetInstalledTime = targetInstalledTime;
	client->targetInstalled = false;

	char buffer[11];
	sprintf(buffer, "/client%03d", client->id);

	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(ClientMessage);
	if((client->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)) == -1) {
		char errorBuffer[30];
		sprintf(errorBuffer, "Could not create client %03d\n", client->id);
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
		sprintf(errorBuffer, "Could not destroy client %03d\n", client->id);
		perror(errorBuffer);
	}

	free(client);
}

void* client_launch(void* client) {
	Client* client1 = (Client*) client;
	ClientMessage clientMessage;
	struct timespec time;
	time.tv_sec = client1->targetInstalledTime;
	time.tv_nsec = 0;
	ssize_t result;

	while(client1->packagesToReceive > 0){
		result = mq_timedreceive(client1->msgQueueID, (char*) &clientMessage, sizeof(ClientMessage), 0, time);
		if(result < 0){
			if(errno != ETIMEDOUT){
				check(result, "mq_receive failed\n");
			}
			else{
				if(client1->targetInstalled){
					client1->targetInstalled = false;
					LOG_INFO("Client %03d target removed\n", client1->id);
				}
			}
		}
		else{
			process_message(client1, &clientMessage);
		}
	}
	LOG_INFO("Client %03d end of delivery\n", client1->id);
	pthread_exit(0);
}

void client_sendMessage(Client* client, ClientMessage* message) {
	check(mq_send(client->msgQueueID, (const char*) message, sizeof(ClientMessage), 0), "mq_send failed");
}

void process_message(Client* client, ClientMessage* message) {
	switch(message->type){
		case DRONE_PUT_TARGET:
			client->targetInstalled = true;
			LOG_INFO("Client %03d target installed\n", client->id);
			break;
		case DRONE_DELIVERY_SUCCESS:
			--(client->packagesToReceive);
			LOG_INFO("Client %03d package received\n", client->id);
			break;
		case DRONE_DELIVERY_FAILURE:
			LOG_INFO("Client %03d package not received\n", client->id);
			break;
		case DRONE_DELIVERY_FINAL_FAILURE:
			--(client->packagesToReceive);
			LOG_INFO("Client %03d package not received for the last time\n", client->id);
			break;
		default:
			break;
	}
}
