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

static void process_message(Client* client, ClientMessage* message);

Client* client_constructor(unsigned int id, unsigned int distance, unsigned int packagesToReceive) {
	Client* client = (Client*) malloc(sizeof(Client));
	client->id = id;
	client->distance = distance;
	client->packagesToReceive = packagesToReceive;
	client->packagesFailed = false;
	client->targetInstalled = false;
	client->nbrPackageFlying = 0;
	check(pthread_mutex_init(&(client->targetMutex), NULL),"Client: pthread_mutex_init failed");

	char buffer[11];
	sprintf(buffer, "/client%03d", client->id);
	struct mq_attr attr;
	attr.mq_curmsgs = 0;
	attr.mq_maxmsg = 10;
	attr.mq_flags = 0;
	attr.mq_msgsize = sizeof(ClientMessage);
	check((client->msgQueueID = mq_open(buffer, O_RDWR | O_CREAT, 0660, &attr)),"Client: mq_open failed");
	check(mq_unlink(buffer),"Client: mq_unlink failed");

	return client;
}

void client_free(Client* client) {
	check(pthread_mutex_destroy(&(client->targetMutex)), "Client: pthread_mutex_destroy failed");
	check(mq_close(client->msgQueueID), "Client: mq_close failed");
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

	if(client->packagesFailed){
		dashboardMessage.state = D_CLIENT_FINISHED_INCOMPLETE;
	}
	else{
		dashboardMessage.state = D_CLIENT_FINISHED_COMPLETE;
	}
	dashboard_sendMessage(global_dashboard, &dashboardMessage);
	LOG_INFO("Client %03d end of delivery", client->id);
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
			++(client->nbrPackageFlying);
			if (client->nbrPackageFlying == 1 && (rand() % 5) != 0) { // Sometimes the client isn't here.
				pthread_mutex_lock(&(client->targetMutex));
				client->targetInstalled = true;
				pthread_mutex_unlock(&(client->targetMutex));

				dashboardMessage.state = D_CLIENT_TARGET_OUT;
				dashboard_sendMessage(global_dashboard, &dashboardMessage);
			}
			else {
				dashboardMessage.state = D_CLIENT_ABSENT;
				dashboard_sendMessage(global_dashboard, &dashboardMessage);
			}

			LOG_INFO("Client %03d target installed", client->id);
			break;
		case DRONE_DELIVERY_SUCCESS:
			--(client->nbrPackageFlying);
			if (client->nbrPackageFlying == 0) {
				pthread_mutex_lock(&(client->targetMutex));
				client->targetInstalled = false;
				pthread_mutex_unlock(&(client->targetMutex));
			}
			--(client->packagesToReceive);

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d package received", client->id);
			break;
		case DRONE_DELIVERY_FAILURE:
			--(client->nbrPackageFlying);
			if (client->nbrPackageFlying == 0) {
				pthread_mutex_lock(&(client->targetMutex));
				client->targetInstalled = false;
				pthread_mutex_unlock(&(client->targetMutex));
			}

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d package not received", client->id);
			break;
		case DRONE_DELIVERY_FINAL_FAILURE:
			pthread_mutex_lock(&(client->targetMutex));
			client->targetInstalled = false;
			pthread_mutex_unlock(&(client->targetMutex));
			--(client->packagesToReceive);
			client->packagesFailed = true;

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d package not received for the last time", client->id);
			break;
		case MOTHERSHIP_UNABLE_TO_SEND_PACKAGE:
			pthread_mutex_lock(&(client->targetMutex));
			client->targetInstalled = false;
			pthread_mutex_unlock(&(client->targetMutex));
			--(client->packagesToReceive);
			client->packagesFailed = true;

			dashboardMessage.state = D_CLIENT_WAITING;
			dashboard_sendMessage(global_dashboard, &dashboardMessage);
			LOG_INFO("Client %03d can't be delivered", client->id);
		default:
			break;
	}
}
