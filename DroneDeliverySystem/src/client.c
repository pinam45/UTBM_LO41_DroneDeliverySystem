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
#include "dashboard.h"

static struct mq_attr attr;

static void process_message(Client* client, ClientMessage* message);

Client* client_constructor(unsigned int id, unsigned int distance, unsigned int packagesToReceive) {
	Client* client = (Client*) malloc(sizeof(Client));
	client->id = id;
	client->distance = distance;
	client->packagesToReceive = packagesToReceive;
	client->targetInstalled = false;
	check(pthread_mutex_init(&(client->targetMutex), NULL),"Client: pthread_mutex_init failed");

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
	int result = pthread_mutex_destroy(&(client->targetMutex));
	if(result < 0) {
		char errorBuffer[50];
		sprintf(errorBuffer, "client %03d: pthread_mutex_destroy failed\n", client->id);
		check(result, errorBuffer);
	}
	result = mq_close(client->msgQueueID);
	if(result < 0) {
		char errorBuffer[30];
		sprintf(errorBuffer, "client %03d: mq_close failed\n", client->id);
		check(result, errorBuffer);
	}
	free(client);
}

void* client_launch(Client* client) {
	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_CLIENT;
	dashboardMessage.number = client->id;
	dashboardMessage.state = D_CLIENT_WAITING;
	dashboard_sendMessage(global_dashboard, &dashboardMessage);

	ClientMessage clientMessage;
	while(client->packagesToReceive > 0) {
		check((int) mq_receive(client->msgQueueID, (char*) &clientMessage, sizeof(ClientMessage), 0),
		      "Client: mq_receive failed");
		process_message(client, &clientMessage);
	}

	dashboardMessage.state = D_CLIENT_FINISHED;
	dashboard_sendMessage(global_dashboard, &dashboardMessage);
	LOG_INFO("Client %03d end of delivery\n", client->id);
	pthread_exit(0);
}

void client_sendMessage(Client* client, ClientMessage* message) {
	check(mq_send(client->msgQueueID, (const char*) message, sizeof(ClientMessage), 0), "mq_send failed");
}

void process_message(Client* client, ClientMessage* message) {
	DashboardMessage dashboardMessage;
	dashboardMessage.type = D_CLIENT;
	dashboardMessage.number = client->id;

	switch(message->type) {
		case DRONE_PUT_TARGET:
			pthread_mutex_lock(&(client->targetMutex));
			client->targetInstalled = true;
			pthread_mutex_unlock(&(client->targetMutex));

			dashboardMessage.state = D_CLIENT_TARGET_OUT;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d target installed\n", client->id);
			break;
		case DRONE_DELIVERY_SUCCESS:
			pthread_mutex_lock(&(client->targetMutex));
			client->targetInstalled = false;
			pthread_mutex_unlock(&(client->targetMutex));
			--(client->packagesToReceive);

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d package received\n", client->id);
			break;
		case DRONE_DELIVERY_FAILURE:
			pthread_mutex_lock(&(client->targetMutex));
			client->targetInstalled = false;
			pthread_mutex_unlock(&(client->targetMutex));

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d package not received\n", client->id);
			break;
		case DRONE_DELIVERY_FINAL_FAILURE:
			pthread_mutex_lock(&(client->targetMutex));
			client->targetInstalled = false;
			pthread_mutex_unlock(&(client->targetMutex));
			--(client->packagesToReceive);

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d package not received for the last time\n", client->id);
			break;
		case MOTHERSHIP_UNABLE_TO_SEND_PACKAGE:
			pthread_mutex_lock(&(client->targetMutex));
			client->targetInstalled = false;
			pthread_mutex_unlock(&(client->targetMutex));
			--(client->packagesToReceive);

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d can't be delivered.\n", client->id);
		default:
			break;
	}
}
